ZigBee Smart Energy Sample Applications

These sample applications demonstrate basic ZigBee Smart Energy functionality
between an Energy Services Interface (ESI) and a Programmable Control 
Thermostat (PCT).

At startup the devices will restart network services if they are joined.

If they are not joined, then the following procedure should be used to
join them.

1.  Press Button 0 on the ESI to allow joining.
2.  Press Button 0 on the PCT to initiate joining.

By default the images have NOT been configured with certificates and support
for ECC and the Key Establishment cluster.  Please see application note 
"AN714 - Smart Energy ECC-Enabled Device Setup Process" for details of how to do
this.

The PCT will query the ESI for a new price every minute and display it on the 
CLI.  To alternate the price between a normal and sale price, press button 1
on the ESI.

All application code is contained in the _callbacks.c files within each
application directory.

To use each application:

   1. Load the included application configuration file (i.e., the .isc file)
      into Simplicity Studio.

   2. Enter a new name for the application in the pop-up window.

   3. Generate the application's header and project/workspace files by
      clicking on the "Generate" button in Simplicity Studio.  The application
      files will be generated in the app/builder/<name> directory in the stack
      installation location.

   4. Load the generated project file into the appropriate compiler and build
      the project.

   5. Load the binary image onto a device using Simplicity Studio.
