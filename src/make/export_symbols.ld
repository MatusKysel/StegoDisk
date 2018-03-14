{
global:
    extern "C++" {
		stego_disk::StegoStorage::*;
		stego_disk::EncoderFactory::*;
		stego_disk::PermutationFactory::*;
		stego_disk::PermutationFactory::*;
		FuseService::*;
		"LoggerVerbosityLevel";
		Logger::*;
    };
/* PSTODO this make symbols local. Do we want to realy hide them or this is sufficient solution? */
local:
    *;
};
