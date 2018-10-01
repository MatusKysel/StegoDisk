#ifndef STEGODISK_UTILS_EXCEPTIONS_H_
#define STEGODISK_UTILS_EXCEPTIONS_H_

#include <exception>
#include <stdexcept>
#include <string>

#include "api_mask.h"
#include "stego-disk_export.h"

#define DEFINE_GETTER(member_name) \
	auto get_ ## member_name() const { return member_name ## _; }
#define DEFINE_STRING_GETTER(member_name) \
	const char * get_ ## member_name() const { return member_name ## _.c_str(); }

namespace stego_disk::exception {

class STEGO_DISK_EXPORT InvalidState : public std::runtime_error {
public:
	enum class ComponentState {
		notInitialized,
		notSetted,
		notActive,
		isActive,
		notConfigured,
		notOpened,
		notLoaded,
		notApplied,
		stopped,
	};

	enum class Component {
		storage,
		encoder,
		virtualStorage,
		permutation,
		threadPool,
		file
	};

	enum class Operation {
		loadVirtualStorage,
		saveVirtualStorage,
		setEncoderArg,
		applyEncoder,
		getCapacity,
		loadStegoStrorage,
		saveStegoStrorage,
		ioStegoStorage,
		ioVirtualStorage,
		commonPermuteInputCheck,
		enqueue,
		save,
		embedBufferUsingEncoder,
	};

public:
	InvalidState(const Operation & operation, const Component & component,
			     const ComponentState & state);

	DEFINE_GETTER(state);
	DEFINE_GETTER(component);
	DEFINE_GETTER(operation);

	virtual ~InvalidState() {}

private:
	const Operation operation_;
	const Component component_;
	const ComponentState state_;
};

using Component = InvalidState::Component;
using ComponentState = InvalidState::ComponentState;
using Operation = InvalidState::Operation;

std::string to_string(const InvalidState::Component & component);
std::string to_string(const InvalidState::ComponentState & state);
std::string to_string(const InvalidState::Operation & operation);



class STEGO_DISK_EXPORT MissingDefault : public std::runtime_error {
public:
	MissingDefault(const std::string & element) :
		std::runtime_error("Missing default value for '"+element+"'"),
		element_{element}
		{}

	DEFINE_STRING_GETTER(element);

	virtual ~MissingDefault() {}

private:
	const std::string element_;
};

class STEGO_DISK_EXPORT EmptyArgument : public std::invalid_argument {
public:
	EmptyArgument(const std::string & argument) :
		std::invalid_argument("Argument '"+argument+"' can not be empty"),
		argument_{argument}
		{}

	DEFINE_STRING_GETTER(argument);

	virtual ~EmptyArgument() {}

private:
	const std::string argument_;
};

class STEGO_DISK_EXPORT NullptrArgument : public std::invalid_argument {
public:
	NullptrArgument(const std::string & argument) :
		std::invalid_argument("Argument '"+argument+"' can not be nullptr"),
		argument_{argument}
		{}

	DEFINE_STRING_GETTER(argument);

	virtual ~NullptrArgument() {}

private:
	const std::string argument_;
};

class STEGO_DISK_EXPORT ZeroAllocatedSize : public std::bad_array_new_length {
public:
	ZeroAllocatedSize() {}

	virtual const char * what() const noexcept { return "Size of new created "
		                                                "buffer is zero"; }

	virtual ~ZeroAllocatedSize() {}
};

class STEGO_DISK_EXPORT EmptyMember : public std::length_error {
public:
	EmptyMember(const std::string & member) :
		std::length_error("Member '"+member+"' can not be empty"),
		member_{member}
		{}

	DEFINE_STRING_GETTER(member);

	virtual ~EmptyMember() {}

private:
	const std::string member_;
};

class STEGO_DISK_EXPORT ErrorOpenFIle : public std::runtime_error {
public:
	ErrorOpenFIle(const std::string & path) :
		std::runtime_error("File '"+path+"' can not be opened"),
		path_{path},
		reason_{""}
		{}
	ErrorOpenFIle(const std::string & path, const std::string & reason) :
		std::runtime_error("File '"+path+"' can not be opened bacouse of '"+reason+"'"),
		path_{path},
		reason_{reason}
		{}
	
	DEFINE_STRING_GETTER(path);
	DEFINE_STRING_GETTER(reason);

	virtual ~ErrorOpenFIle() {}

private:
	const std::string path_;
	const std::string reason_;
};

class STEGO_DISK_EXPORT ParseError : public std::runtime_error {
public:
	ParseError(const std::string & config, const std::string & error) :
		std::runtime_error("File '"+config+"' can not be parsed for '"+
				            error+"'"),
		config_{config},
		error_{error}
		{}
	
	DEFINE_STRING_GETTER(config);
	DEFINE_STRING_GETTER(error);

	virtual ~ParseError() {}

private:
	const std::string config_;
	const std::string error_;
};

class STEGO_DISK_EXPORT ExecFailed : public std::runtime_error {
public:
	ExecFailed(const std::string & command) :
		std::runtime_error("Failed to execute command '"+command+"'"),
		command_{command}
		{}

	DEFINE_STRING_GETTER(command);

	virtual ~ExecFailed() {}

private:
	const std::string command_;
};

class STEGO_DISK_EXPORT HashSizeTooSmall : public std::runtime_error {
public:
    HashSizeTooSmall() :
		std::runtime_error("Hash size is too small")
		{}

	virtual ~HashSizeTooSmall() {}
};

class STEGO_DISK_EXPORT PermutationFailed : public std::runtime_error {
public:
    PermutationFailed() :
		std::runtime_error("Permuted index calculation failed")
		{}

	virtual ~PermutationFailed() {}
};

class STEGO_DISK_EXPORT FuctionNotImplementad : public std::exception {
public:
    FuctionNotImplementad() {}

	virtual const char * what() const noexcept { return "This function is not "
		                                                "implemented yet"; }

	virtual ~FuctionNotImplementad() {}
};

class STEGO_DISK_EXPORT IoError : public std::runtime_error {
public:
	IoError(const std::string & file) :
		std::runtime_error("IO for file '"+file+"' failed"),
		file_{file}
		{}
	
	DEFINE_STRING_GETTER(file);

	virtual ~IoError() {}

private:
	const std::string file_;
};
}

#endif // STEGODISK_UTILS_EXCEPTIONS_H_
