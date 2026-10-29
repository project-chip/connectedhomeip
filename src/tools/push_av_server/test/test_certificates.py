"""Tests for CAHierarchy certificate validity windows.

A DUT checks provisioned certificates against its own clock, so a certificate
must already be valid when it arrives even if the DUT clock trails ours.
"""

import datetime
import types

import certificates
import pytest
from cryptography.hazmat.primitives.asymmetric import ec

FIXED_NOW = datetime.datetime(2030, 6, 1, 12, 0, 0, tzinfo=datetime.UTC)


@pytest.fixture
def ca(tmp_path, monkeypatch):
    clock = types.SimpleNamespace(now=lambda tz=None: FIXED_NOW)
    monkeypatch.setattr(certificates, "datetime", types.SimpleNamespace(
        datetime=clock, timedelta=datetime.timedelta, UTC=datetime.UTC))
    return certificates.CAHierarchy(tmp_path, "server", "server")


def test_root_certificate_starts_a_day_early(ca):
    assert ca.root_cert.not_valid_before_utc == FIXED_NOW - datetime.timedelta(days=1)
    assert ca.root_cert.not_valid_after_utc == FIXED_NOW + ca.default_ca_duration


def test_leaf_certificate_starts_a_day_early(ca):
    duration = datetime.timedelta(hours=1)
    leaf = ca._sign_cert("dut", ec.generate_private_key(ec.SECP256R1()).public_key(), duration)
    assert leaf.not_valid_before_utc == FIXED_NOW - datetime.timedelta(days=1)
    assert leaf.not_valid_after_utc == FIXED_NOW + duration
