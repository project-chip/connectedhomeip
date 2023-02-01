## Creating a custom matter_idl generator

The matter_idl tool can be used to generate arbitrary code based on the Matter
data model schemas. To create a custom generator that lives outside of the
Matter SDK tree, follow the design pattern of
scripts/py_matter_idl/examples/matter_idl_plugin:

1. Create a directory exactly named "matter_idl_plugin".
2. Add an `__init__.py` under "matter_idl_plugin" implementing a subclass of
   `CodeGenerator` named `CustomGenerator`.
3. Have CustomGenerator load jinja templates, also under the "matter_idl_plugin"
   subdirectory.
4. Execute the `codegen.py` script from the parent directory of
   "matter_idl_plugin".

```
cd scripts/py_matter_idl/examples
../../codegen.py --generator custom ../../../src/controller/data_model/controller-clusters.matter
```
