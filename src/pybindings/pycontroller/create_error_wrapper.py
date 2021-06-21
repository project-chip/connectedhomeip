import sys
import re
import os
import argparse


class ErrorCode(object):
    def __init__(self, error_name, error_code, min_error_code):
        self.name = error_name.replace('_', ' ').title().replace(' ', '')
        self.message = error_name
        self.code = int(error_code) + min_error_code

    def __str__(self):
        return '<ErrorCode name="{}" code="{}">'.format(self.name, self.code)

    def __repr__(self):
        return '<ErrorCode name="{}" code={}>'.format(self.name, self.code)

    def create_exception_class(self):
        return '''
struct <name> : public std::exception
{
    const char * what () const throw ()
    {
    	return "<message>";
    }
};
'''.replace('<name>', self.name).replace('<message>', self.message)

    def create_py_bind(self, module_name):
        return 'py::register_exception<chip::PythonBindings::{}>({}, "{}");'.format(self.name, module_name, self.name)

    def create_else_if(self, var_name):
        return '''
    else if (<var_name> == <error_code>) {
        throw chip::PythonBindings::<name>();
    }'''.replace('<var_name>', var_name).replace('<error_code>', str(self.code)).replace('<name>', self.name)


def generate_header(error_codes):
    structs = "".join([x.create_exception_class() for x in error_codes])
    return '''
#include <exception>
#include <core/CHIPError.h>

namespace chip {
namespace PythonBindings {
<structs>

void CHIPErrorToException(CHIP_ERROR err);
}
}
'''.replace('<structs>', structs)


def generate_cpp(error_codes):
    exception_binding = '\n\t'.join(
        [x.create_py_bind('M("ChipExceptions")') for x in error_codes])
    elseifs = ''.join([x.create_else_if('err') for x in error_codes])
    return '''
#include "pybind11/pybind11.h"
#include "CHIPErrorToExceptionBindings.h"
#include <functional>

namespace py = pybind11;

void CHIPErrorToException(CHIP_ERROR err) {
    if (err == CHIP_NO_ERROR) {
        //Do Nothing
    }<elseifs>
}

void bind_CHIPController_ChipExceptions(std::function< pybind11::module &(std::string const &namespace_) > &M)
{
    M("ChipExceptions").def("CHIPErrorToException", &CHIPErrorToException, "Converts a CHIP_ERROR (int) to an Exception");

    <exception_binding>
}
'''.replace('<exception_binding>', exception_binding).replace('<elseifs>', elseifs)


def create_error_bindings(error_path, config_path, header_out, cpp_out):
    error_header = None
    config_header = None

    with open(error_path, 'r') as f:
        error_header = f.read()

    with open(config_path, 'r') as f:
        config_header = f.read()

    if error_header != None and config_header != None:
        min_err_re = re.compile(r'CHIP_CONFIG_CORE_ERROR_MIN (\d+)')
        matches = min_err_re.findall(config_header)

        min_err = -1

        if len(matches) >= 1:
            min_err = int(matches[0])

        if min_err != -1:
            error_code_re = re.compile(r'#define (\S+).*_CHIP_ERROR\((\d+)\)')
            error_code_tuples = error_code_re.findall(error_header)
            error_codes = [ErrorCode(x[0], x[1], min_err)
                           for x in error_code_tuples]
            header = generate_header(error_codes)
            cpp = generate_cpp(error_codes)

            print("Creating Header file: {}".format(header_out))
            with open(header_out, 'w') as f:
                f.write(header)

            print("Creating CPP file {}".format(cpp_out))
            with open(cpp_out, 'w') as f:
                f.write(cpp)

        else:
            print("Could not find min error value")
            sys.exit(1)
    else:
        print("Unable to open required files :(")
        sys.exit(1)


def main():
    p = argparse.ArgumentParser()

    p.add_argument('--output_cpp_file', required=True,
                   help='Output location for .cpp file')
    p.add_argument('--output_header_file', required=True,
                   help='Output location for .h file')
    p.add_argument('--error_header', required=False,
                   help='Location of Error Header file (ex CHIPError.h)')
    p.add_argument('--config_header', required=False,
                   help='Location of Config Header file (ex CHIPConfig.h)')

    args = p.parse_args()

    output_cpp_file = args.output_cpp_file
    output_header_file = args.output_header_file

    error_path = '../../lib/core/CHIPError.h'
    config_path = '../../lib/core/CHIPConfig.h'

    if args.error_header != None and args.error_header != "":
        error_path = os.path.abspath(args.error_header)

    if not os.path.exists(error_path):
        print("Error Header Path {} does not exist".format(error_path))
        sys.exit(1)

    if args.config_header != None and args.config_header != "":
        config_path = os.path.abspath(args.config_header)

    if not os.path.exists(config_path):
        print("Config Header Path {} does not exist!".format(config_path))

    create_error_bindings(error_path, config_path,
                          output_header_file, output_cpp_file)


if __name__ == '__main__':
    main()
