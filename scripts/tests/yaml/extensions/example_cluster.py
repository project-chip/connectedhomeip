#
# SPDX-FileCopyrightText: 2023 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0


from matter_yamltests.pseudo_clusters.pseudo_cluster import PseudoCluster


class example_cluster(PseudoCluster):
    name = 'ExampleCluster'

    async def CommandSuccess(self, request):
        return {}

    async def CommandError(self, request):
        return {'error': 'UNSUPPORTED_COMMAND'}

    async def CommandWithReturnValues(self, request):
        rv = {
            'BooleanTrue': True,
            'BooleanFalse': False,
            'PositiveNumber': 123456789,
            'NegativeNumber': -123456789,
            'ListOfBoolean': [True, False, True],
            'Struct': {
                'boolean': True,
                'number': 1,
                'struct': {
                    'boolean': False,
                    'number': 2,
                },
                'list': [
                    {
                        'boolean': True,
                        'number': 1,
                    },
                    {
                        'boolean': False,
                        'number': 3,
                    },
                ]
            }
        }
        return {'value': rv}

    async def CommandWithInputValues(self, request):
        try:
            arg1 = int(self._get_argument_value(request, 'Argument1'))
            arg2 = int(self._get_argument_value(request, 'Argument2'))
        except NameError:
            return {'error': 'INVALID_COMMAND'}

        rv = {'ReturnValue': arg1 + arg2}
        return {'value': rv}

    def _get_argument_value(self, request, name):
        for argument in request.arguments['values']:
            if argument['name'] == name:
                return argument['value']

        raise NameError
