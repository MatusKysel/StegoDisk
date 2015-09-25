#ifndef STEGODISK_FILEMANAGEMENT_CARRIERFILESMANAGERDELEGATE_H_
#define STEGODISK_FILEMANAGEMENT_CARRIERFILESMANAGERDELEGATE_H_

namespace stego_disk {

enum StegoLoadingStage {
    CountingCapacity = 0,
    LoadingFiles = 1,
    SavingFiles = 2
};

class CarrierFilesManagerDelegate
{
public:
    virtual void FmProgressUpdated(int progress, int max, StegoLoadingStage stage) = 0;
};

} // stego_disk

#endif // STEGODISK_FILEMANAGEMENT_CARRIERFILESMANAGERDELEGATE_H_
