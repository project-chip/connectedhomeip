## Creating a custom matter_idl generator

The matter_idl tool can be used to generate arbitrary code based on the Matter
data model schemas. To create a custom generator that lives outside of the
Matter SDK tree, follow the design pattern of
scripts/py_matter_idl/examples/matter_idl_plugin:

1. Create a directory for your python generator module, for example
   "matter_idl_plugin".
2. Add an `__init__.py` under "matter_idl_plugin" implementing a subclass of
   `CodeGenerator` named `CustomGenerator`.
3. Have `CustomGenerator` load jinja templates, also under the
   "matter_idl_plugin" subdirectory.
4. Execute the `codegen.py` script passing the path to the parent directory of
   "matter_idl_plugin" via
   `--generator custom:<plugin_path>:<plugin_module_name>` argument and package
   name like `--option package:com.example.matter.proto`

```
# From top-of-tree in this example
./scripts/codegen.py \
  --generator custom:./scripts/py_matter_idl/examples:matter_idl_plugin \
  --option package:com.example.matter.proto \
  ./src/controller/data_model/controller-clusters.matter
```
