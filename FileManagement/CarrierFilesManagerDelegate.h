#ifndef CARRIERFILESMANAGERDELEGATE_H
#define CARRIERFILESMANAGERDELEGATE_H

namespace StegoDisk {

enum StegoLoadingStage {
    CountingCapacity = 0,
    LoadingFiles = 1,
    SavingFiles = 2
};

class CarrierFilesManagerDelegate
{   
public:
    virtual void cfmProgressUpdated(int progress, int max, StegoLoadingStage stage) = 0;
};

}

#endif // CARRIERFILEMANAGERDELEGATE_H
