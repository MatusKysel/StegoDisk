#include "exceptions.h"

namespace stego_disk {
namespace exception {

InvalidState::InvalidState(const Operation & operation, const Component & component, const ComponentState & state) :
	std::runtime_error{"Required operation '"+to_string(operation)+"' can not be applied because of current state '"+to_string(state)+"' of the component '"+to_string(component)+"'"},
	operation_{operation},
	component_{component},
	state_{state}
{}

std::string to_string(const InvalidState::Component & component) {
	switch (component) {
		case InvalidState::Component::storage:
			return "Storage";
		case InvalidState::Component::encoder:
			return "Encoder";
		case InvalidState::Component::virtualStorage:
			return "Virtual storage";
		case InvalidState::Component::permutation:
			return "Permutation";
		case InvalidState::Component::threadPool:
			return "Thread pool";
		case InvalidState::Component::file:
			return "File";
	}
	throw std::invalid_argument{"component"};
}

std::string to_string(const InvalidState::ComponentState & state) {
	switch (state) {
		case InvalidState::ComponentState::notInitialized:
			return "Not initialized";
		case InvalidState::ComponentState::notSetted:
			return "Not setted";
		case InvalidState::ComponentState::notActive:
			return "Not active";
		case InvalidState::ComponentState::isActive:
			return "Is active";
		case InvalidState::ComponentState::notConfigured:
			return "Not configured";
		case InvalidState::ComponentState::notOpened:
			return "Not opened";
		case InvalidState::ComponentState::notLoaded:
			return "Not loaded";
		case InvalidState::ComponentState::notApplied:
			return "Not applied";
		case InvalidState::ComponentState::stopped:
			return "Stopped";
	}
	throw std::invalid_argument{"state"};
}

std::string to_string(const InvalidState::Operation & operation) {
	switch (operation) {
		case InvalidState::Operation::loadVirtualStorage:
			return "Load virtual storage";
		case InvalidState::Operation::saveVirtualStorage:
			return "Save virtual storage";
		case InvalidState::Operation::setEncoderArg:
			return "Set encoder argument";
		case InvalidState::Operation::applyEncoder:
			return "Apply encoder";
		case InvalidState::Operation::getCapacity:
			return "Get capacity";
		case InvalidState::Operation::loadStegoStrorage:
			return "Load stego storage";
		case InvalidState::Operation::saveStegoStrorage:
			return "Save stego storage";
		case InvalidState::Operation::ioStegoStorage:
			return "IO operation on stego storage";
		case InvalidState::Operation::ioVirtualStorage:
			return "IO operation on virtual storage";
		case InvalidState::Operation::commonPermuteInputCheck:
			return "Common permutate input check";
		case InvalidState::Operation::enqueue:
			return "Enqueue";
		case InvalidState::Operation::save:
			return "Save";
		case InvalidState::Operation::embedBufferUsingEncoder:
			return "Embed buffer using encoder";
	}
	throw std::invalid_argument{"operation"};
}

} // exception
} // stego_disk
