#include "app/clusters/ota-requestor/ota-requestor-driver.h"

class LinuxOTARequestorDriver : public OTARequestorDriver {

    // Virtual functions from OTARequestorDriver -- start

    // A call into the application logic to give it a chance to allow or stop the Requestor
    // from proceeding with actual image download. Returning TRUE will allow the download
    // to proceed, returning FALSE will abort the download process.
    bool CheckImageDownloadAllowed() {return true;}

    // Notify the application that the download is complete and the image can be applied
    void ImageDownloadComplete() {}

    // Virtual functions from OTARequestorDriver -- end
};
