#include "file.h"

#include "api_mask.h"
#include "utils/include_fs_library.h"

#ifndef HAS_FILESYSTEM_LIBRARY
#  include "utils/file_win.inc"
#  include "utils/file_unix.inc"
#else //HAS_FILESYSTEM_LIBRARY

namespace stego_disk {

std::vector<File> File::GetFilesInDir(const std::string &directory, const std::string &filter)
{
	std::regex rx(".*\\.(" + filter + ")$", std::regex_constants::icase);

	std::vector<File> ret;
	for (auto &i: fs::recursive_directory_iterator(directory)) {
		const fs::path & i_path = i.path();
		//PSTODO skusit neplatne symlinky
		if (fs::status(i).type() != fs::file_type::regular) //PSTODO chceme aj nejake ine?
			continue;

		auto parent_path = i_path.parent_path().string();
		auto filename = i_path.filename().string();

		if (filter == "")
		{
			ret.emplace_back(File(parent_path, filename));
		}
		else
		{
			if (std::regex_match(filename, rx))
			{
				ret.emplace_back(File(parent_path, filename));
			}
		}
	}
	return ret;
}

} // stego_disk

#endif //HAS_FILESYSTEM_LIBRARY
