"""Retained regression tests for the explicitly opted-in one-shot read contract."""

import argparse
import asyncio
import json
import runpy
import sys
import unittest
from collections.abc import Callable
from types import SimpleNamespace
from typing import Any
from unittest.mock import AsyncMock, Mock, patch

from matter.ChipDeviceCtrl import ChipDeviceController
from matter.clusters.Attribute import AsyncReadTransaction, ReadOperationOptions
from matter.exceptions import ChipStackError


class TestReadOperation(unittest.IsolatedAsyncioTestCase):
    async def test_native_runner_invokes_both_retained_regression_groups(self):
        read_cases = AsyncMock(return_value=[{'case': 'read'}])
        restoration_cases = AsyncMock(return_value=[{'case': 'restoration'}])
        checker = SimpleNamespace(default_controller=object(), dut_node_id=1)
        pause, resume, inject = Mock(), Mock(), Mock()
        with patch.dict(run_native_read_regressions.__globals__, {
                'exercise_native_read_operation_contract': read_cases,
                'exercise_native_idm_restoration_contract': restoration_cases}):
            rows = await run_native_read_regressions(checker, pause, resume, inject)
        self.assertEqual(rows, [{'case': 'read'}, {'case': 'restoration'}])
        read_cases.assert_awaited_once_with(checker.default_controller, 1, pause, resume, inject)
        restoration_cases.assert_awaited_once_with(checker, pause, resume)

    def transaction(self):
        loop = asyncio.get_running_loop()
        transaction = AsyncReadTransaction(loop.create_future(), loop, None, False)
        transaction.AbortRead = Mock()
        return transaction

    async def test_completed_read_does_not_abort(self):
        transaction = self.transaction()
        transaction._future.set_result(transaction)
        await transaction.AwaitCompletion()
        transaction.AbortRead.assert_not_called()

    async def test_completion_during_startup_cannot_reinstall_pointer(self):
        transaction = self.transaction()
        transaction._handleDone = Mock()
        transaction.handleDone()
        transaction.SetClientObjPointers(object())
        transaction.handleDone()
        delivered = asyncio.Event()
        asyncio.get_running_loop().call_soon(delivered.set)
        await delivered.wait()
        self.assertIsNone(transaction._pReadClient)
        transaction._handleDone.assert_called_once()

    async def test_controller_rejects_subscription_options_before_startup(self):
        controller = SimpleNamespace(CheckIsActive=Mock(), GetConnectedDevice=AsyncMock())
        with self.assertRaises(ValueError):
            await ChipDeviceController.Read(controller, 1, reportInterval=(1, 2), readOperation=ReadOperationOptions(100))
        controller.GetConnectedDevice.assert_not_called()

    async def test_controller_deadline_includes_session_startup(self):
        started = asyncio.Event()
        gate = asyncio.get_running_loop().create_future()

        async def connect(*args, **kwargs):
            started.set()
            return await gate

        async def retry(node, operation):
            return await operation()

        controller = SimpleNamespace(CheckIsActive=Mock(), GetConnectedDevice=connect, _run_with_session_retry=retry)
        with self.assertRaises(TimeoutError):
            await ChipDeviceController.Read(controller, 1, readOperation=ReadOperationOptions(1))
        self.assertTrue(started.is_set())
        self.assertTrue(gate.cancelled())

    async def test_cancellation_keeps_future_alive_until_completion(self):
        transaction = self.transaction()
        started = asyncio.Event()
        aborted = asyncio.Event()
        transaction.AbortRead.side_effect = aborted.set
        first = None

        async def read():
            nonlocal first
            started.set()
            try:
                await transaction.AwaitCompletion()
            except asyncio.CancelledError as error:
                first = error
                raise

        task = asyncio.create_task(read())
        await started.wait()
        task.cancel('first')
        await aborted.wait()
        task.cancel('second')
        self.assertFalse(transaction._future.cancelled())
        self.assertFalse(task.done())
        transaction._future.set_result(transaction)
        with self.assertRaises(asyncio.CancelledError) as result:
            await task
        self.assertIs(result.exception, first)
        self.assertEqual(result.exception.args, ('first',))
        transaction.AbortRead.assert_called_once()

    async def test_deadline_aborts_stalled_read(self):
        transaction = self.transaction()
        transaction.AbortRead.side_effect = lambda: transaction._future.set_result(transaction)
        with self.assertRaises(TimeoutError):
            await transaction.AwaitCompletion(asyncio.get_running_loop().time())
        self.assertFalse(transaction._future.cancelled())
        transaction.AbortRead.assert_called_once()

    async def test_abort_failure_preserves_ownership_and_cancellation(self):
        transaction = self.transaction()
        aborted = asyncio.Event()

        def fail_abort():
            aborted.set()
            raise ChipStackError(0x2f)

        transaction.AbortRead.side_effect = fail_abort
        task = asyncio.create_task(transaction.AwaitCompletion())
        asyncio.get_running_loop().call_soon(task.cancel, 'original')
        await aborted.wait()
        self.assertFalse(task.done())
        self.assertFalse(transaction._future.cancelled())
        transaction._future.set_result(transaction)
        with self.assertRaises(asyncio.CancelledError) as result:
            await task
        self.assertEqual(result.exception.args, ('original',))
        self.assertIn('Native read abort failed', result.exception.__notes__[0])
        transaction.AbortRead.assert_called_once()

    def test_invalid_deadline_rejected(self):
        for timeout in (0, -1, True, 1.5):
            with self.subTest(timeout=timeout), self.assertRaises(ValueError):
                ReadOperationOptions(timeout)

    async def test_attribute_and_event_wrappers_forward_explicit_options(self):
        options = ReadOperationOptions(100)
        controller = SimpleNamespace(Read=AsyncMock(return_value=SimpleNamespace(attributes={}, events=[])))
        await ChipDeviceController.ReadAttribute(controller, 1, [], readOperation=options)
        self.assertIs(controller.Read.call_args.kwargs['readOperation'], options)
        await ChipDeviceController.ReadEvent(controller, 1, [], readOperation=options)
        self.assertIs(controller.Read.call_args.kwargs['readOperation'], options)

    async def test_wrappers_omit_options_for_legacy_read_overrides(self):
        controller = SimpleNamespace(Read=AsyncMock(return_value=SimpleNamespace(attributes={}, events=[])))
        await ChipDeviceController.ReadAttribute(controller, 1, [])
        self.assertNotIn('readOperation', controller.Read.call_args.kwargs)
        await ChipDeviceController.ReadEvent(controller, 1, [])
        self.assertNotIn('readOperation', controller.Read.call_args.kwargs)

    async def test_omitted_options_preserve_legacy_future_cancellation(self):
        from unittest.mock import patch

        started = asyncio.Event()
        transactions = []

        async def retry(node, operation):
            return await operation()

        def read(transaction, **kwargs):
            transactions.append(transaction)
            started.set()
            return SimpleNamespace(raise_on_error=Mock())

        controller = SimpleNamespace(CheckIsActive=Mock(), GetConnectedDevice=AsyncMock(
            return_value=SimpleNamespace(deviceProxy=None)), _run_with_session_retry=retry)
        with patch('matter.clusters.Attribute.Read', side_effect=read):
            task = asyncio.create_task(ChipDeviceController.Read(controller, 1))
            await started.wait()
            task.cancel('legacy')
            with self.assertRaises(asyncio.CancelledError):
                await task
        self.assertTrue(transactions[0]._future.cancelled())


async def exercise_native_read_operation_contract(
        controller: ChipDeviceController, node: int, pause_dut: Callable[[], None], resume_dut: Callable[[], None],
        inject_report: Callable[[AsyncReadTransaction], None]) -> list[dict[str, Any]]:
    """Exercise the public read API using an owned software DUT and a native report probe."""
    from unittest.mock import patch

    import matter.clusters as Clusters
    from matter.clusters import Attribute

    attributes = [(0, Clusters.BasicInformation.Attributes.Location)]
    observed = []
    ready = None
    wire_pause = False
    original_read = Attribute.Read
    original_done = Attribute.AsyncReadTransaction._handleDone
    original_await = Attribute.AsyncReadTransaction.AwaitCompletion
    original_begin = Attribute.AsyncReadTransaction.handleReportBegin

    def capture(transaction, *args, **kwargs):
        result = original_read(transaction, *args, **kwargs)
        observed.append({'transaction': transaction, 'done': 0})
        if ready is not None:
            ready.set()
        return result

    def complete(transaction):
        for row in observed:
            if row['transaction'] is transaction:
                row['done'] += 1
                row['future_cancelled'] = transaction._future.cancelled()
                row['pointer_cleared_before_done'] = transaction._pReadClient is None
        original_done(transaction)

    async def settle(transaction, deadline=None):
        try:
            return await original_await(transaction, deadline)
        except BaseException as error:
            for row in observed:
                if row['transaction'] is transaction:
                    row['primary_error'] = error
            raise

    def report_begin(transaction):
        if wire_pause:
            pause_dut()
        original_begin(transaction)

    async def start(timeout_ms, paths=attributes):
        nonlocal ready
        ready = asyncio.Event()
        operation = asyncio.create_task(controller.ReadAttribute(node, paths, readOperation=ReadOperationOptions(timeout_ms)))
        await asyncio.wait_for(ready.wait(), 5)
        ready = None
        return operation, observed[-1]

    def evidence(case, row, **details):
        assert row['done'] == 1, row
        assert row['pointer_cleared_before_done'] and not row['future_cancelled'], row
        return {'case': case, 'done': 1, 'future_cancelled': False, **details}

    results = []
    with patch.object(Attribute, 'Read', capture), patch.object(Attribute.AsyncReadTransaction, '_handleDone', complete), \
            patch.object(Attribute.AsyncReadTransaction, 'AwaitCompletion', settle), \
            patch.object(Attribute.AsyncReadTransaction, 'handleReportBegin', report_begin):
        try:
            operation, row = await start(5000)
            await operation
            row['transaction'].AbortRead()
            results.append(evidence('public-normal-completion-before-abort', row))

            pause_dut()
            operation, row = await start(5000)
            operation.cancel('first public read cancellation')
            asyncio.get_running_loop().call_soon(operation.cancel, 'repeated public read cancellation')
            try:
                await operation
            except asyncio.CancelledError as error:
                assert error is row['primary_error']
                assert error.args == ('first public read cancellation',)
            else:
                raise AssertionError('Public cancellation must propagate')
            results.append(evidence('public-repeated-cancellation', row, original_cancellation_identity=True))
            resume_dut()

            pause_dut()
            operation, row = await start(300)
            try:
                await operation
            except TimeoutError as error:
                assert error is row['primary_error']
            else:
                raise AssertionError('Overall deadline must abort a stalled native read')
            results.append(evidence('public-stalled-wire-response-deadline', row))
            resume_dut()

            pause_dut()
            operation, row = await start(400)
            chunks = 0
            while not operation.done():
                if row['transaction']._pReadClient is not None:
                    inject_report(row['transaction'])
                    chunks += 1
                await asyncio.wait((operation,), timeout=0.075)
            try:
                await operation
            except TimeoutError as error:
                assert error is row['primary_error'] and chunks >= 3
            else:
                raise AssertionError('Non-final reports must not extend the overall deadline')
            results.append(evidence('public-non-final-chunks-overall-deadline', row, chunks=chunks,
                                    native_report_injection_not_wire_origin=True))
            resume_dut()

            subscription = await controller.ReadAttribute(node, attributes, reportInterval=(1, 60))
            subscribed = observed[-1]['transaction']
            try:
                pause_dut()
                operation, row = await start(300)

                def fail_abort():
                    asyncio.get_running_loop().call_soon(resume_dut)
                    subscribed.AbortRead()

                row['transaction'].AbortRead = fail_abort
                try:
                    await operation
                except TimeoutError as error:
                    assert error is row['primary_error']
                    assert any('Native read abort failed' in note for note in error.__notes__)
                else:
                    raise AssertionError('Failed abort must retain the original deadline error')
                results.append(evidence('public-native-abort-failure-safe-settlement', row))
            finally:
                resume_dut()
                subscription.Shutdown()

            wire_pause = True
            operation, row = await start(1000, [()])
            try:
                await operation
            except TimeoutError as error:
                assert error is row['primary_error']
            else:
                raise AssertionError('Pausing a real wildcard report must stall subsequent wire chunks')
            results.append(evidence('public-real-wire-wildcard-stream-stalled-deadline', row,
                                    native_report_injection_not_wire_origin=False))
        finally:
            wire_pause = False
            resume_dut()
    return results


async def exercise_native_idm_restoration_contract(checker: Any, pause_dut: Callable[[], None],
                                                   resume_dut: Callable[[], None]) -> list[dict[str, Any]]:
    """Run the retained IDM algorithm through real reads/writes and cancellation during native readback."""
    from unittest.mock import patch

    from support_modules.idm_support import WritableAttributeInfo, spec_enum_values

    import matter.clusters as Clusters
    from matter.clusters import Attribute
    from matter.testing.spec_parsing import PrebuiltDataModelDirectory, build_xml_clusters

    xml_clusters, _ = build_xml_clusters(PrebuiltDataModelDirectory.k1_7)
    cluster = Clusters.OnOff
    attribute = cluster.Attributes.StartUpOnOff
    xml_cluster = xml_clusters[cluster.id]
    xml_attribute = xml_cluster.attributes[attribute.attribute_id]
    info = WritableAttributeInfo(endpoint_id=1, cluster_id=cluster.id, cluster_name=xml_cluster.name,
                                 attribute_id=attribute.attribute_id, attribute_name=xml_attribute.name, attribute=attribute,
                                 cluster_class=cluster, datatype=xml_attribute.datatype, constraints=xml_attribute.constraints,
                                 enum_values=spec_enum_values(xml_cluster, xml_attribute.datatype))
    original_readback = checker.read_single_attribute
    original_verified = checker.read_single_attribute_check_success
    original_write = checker.default_controller.WriteAttribute
    original_read = Attribute.Read
    original_done = Attribute.AsyncReadTransaction._handleDone
    original_shield = asyncio.shield
    original = await original_verified(endpoint=1, cluster=cluster, attribute=attribute)
    results = []
    checker.constraint_read_timeout_ms = 1000
    checker.constraint_restoration_timeout_ms = 150
    for stall_verification in (False, True):
        ready = asyncio.Event()
        cancellations = []
        transactions = []
        writes = []
        active = False
        verification_count = 0

        def capture(transaction, *args, **kwargs):
            result = original_read(transaction, *args, **kwargs)
            if active:
                transactions.append({'transaction': transaction, 'done': 0})
                ready.set()
            return result

        def complete(transaction):
            for row in transactions:
                if row['transaction'] is transaction:
                    row['done'] += 1
                    row['safe'] = transaction._pReadClient is None and not transaction._future.cancelled()
            original_done(transaction)

        async def shield(operation):
            try:
                return await original_shield(operation)
            except asyncio.CancelledError as error:
                cancellations.append(error)
                raise

        async def readback(**kwargs):
            nonlocal active
            pause_dut()
            active = True
            try:
                return await original_readback(**kwargs)
            finally:
                active = False
                resume_dut()

        async def verified(**kwargs):
            nonlocal active, verification_count
            verification_count += 1
            if verification_count == 1 or not stall_verification:
                return await original_verified(**kwargs)
            pause_dut()
            active = True
            try:
                return await original_verified(**kwargs)
            finally:
                active = False
                resume_dut()

        async def write(**kwargs):
            writes.append(kwargs['attributes'][0][1].value)
            return await original_write(**kwargs)

        with patch.object(checker, 'read_single_attribute', side_effect=readback), \
                patch.object(checker, 'read_single_attribute_check_success', side_effect=verified), \
                patch.object(checker.default_controller, 'WriteAttribute', side_effect=write), \
                patch.object(Attribute, 'Read', capture), patch.object(Attribute.AsyncReadTransaction, '_handleDone', complete), \
                patch('support_modules.idm_support.asyncio.shield', side_effect=shield):
            operation = asyncio.create_task(checker.check_attribute_constraint(info, info.constraints))
            try:
                await asyncio.wait_for(ready.wait(), 5)
                operation.cancel('original IDM native readback cancellation')
                asyncio.get_running_loop().call_soon(operation.cancel, 'repeated IDM cancellation')
                try:
                    await operation
                except asyncio.CancelledError as error:
                    assert error is cancellations[0]
                    assert error.args == ('original IDM native readback cancellation',)
                    if stall_verification:
                        assert any('Restoration verification failed' in note for note in error.__notes__)
                else:
                    raise AssertionError('The original IDM cancellation must remain primary')
            finally:
                resume_dut()
        assert len(writes) == 2 and writes[-1] == original, writes
        assert all(row['done'] == 1 and row['safe'] for row in transactions), transactions
        assert len(transactions) == (2 if stall_verification else 1)
        restored = await original_verified(endpoint=1, cluster=cluster, attribute=attribute)
        assert restored == original
        results.append({'case': 'native-idm-verification-deadline' if stall_verification else 'native-idm-readback-cancellation',
                        'done': 1, 'future_cancelled': False, 'original_cancellation_identity': True,
                        'restoration_diagnostic_retained': stall_verification, 'baseline_verified': True,
                        'no_subsequent_violating_probe': True, 'native_read_transactions_settled_once': len(transactions)})
    return results


async def run_native_read_regressions(checker: Any, pause_dut: Callable[[], None], resume_dut: Callable[[], None],
                                     inject_report: Callable[[Any, int, bool], None]) -> list[dict[str, Any]]:
    """Run both retained native groups against an already commissioned, fixture-owned software DUT.

    The fixture supplies its checker/controller, owned-process pause/resume callbacks,
    and test-only native report injection. These are not ordinary mock unit tests.
    """
    rows = await exercise_native_read_operation_contract(checker.default_controller, checker.dut_node_id,
                                                        pause_dut, resume_dut, inject_report)
    rows.extend(await exercise_native_idm_restoration_contract(checker, pause_dut, resume_dut))
    return rows


def main() -> None:
    """Run unit tests by default, or native regressions with an explicit owned-DUT fixture module.

    --native-fixture supplies a Python file exporting the native_read_fixture async
    context manager. It yields checker, pause_dut, resume_dut and inject_report
    fields and owns commissioning and complete process/stack cleanup.
    """
    if '--native-fixture' not in sys.argv:
        unittest.main()
        return
    parser = argparse.ArgumentParser()
    parser.add_argument('--native-fixture', required=True)
    args = parser.parse_args()
    fixture_factory = runpy.run_path(args.native_fixture)['native_read_fixture']

    async def run() -> None:
        async with fixture_factory() as fixture:
            rows = await run_native_read_regressions(fixture.checker, fixture.pause_dut,
                                                    fixture.resume_dut, fixture.inject_report)
            print(json.dumps({'native_read_regressions': rows}), flush=True)

    asyncio.run(run())


if __name__ == '__main__':
    main()
