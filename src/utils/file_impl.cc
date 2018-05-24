#include "file.h"

#include "api_mask.h"
#include "utils/include_fs_library.h"

#ifndef HAS_FILESYSTEM_LIBRARY
#  include "utils/file_win.inc"
#  include "utils/file_unix.inc"
#else //HAS_FILESYSTEM_LIBRARY

namespace stego_disk {

std::vector<File> File::GetFilesInDir(std::string directory, std::string mask)
{
	std::vector<File> ret;
	for (auto &i: fs::recursive_directory_iterator(directory)) {
		const fs::path & i_path = i.path();
		//PSTODO skusit neplatne symlinky
		if (fs::status(i).type() != fs::file_type::regular) //PSTODO chceme aj nejake ine?
			continue;

		//PSTODO add chceck for mask

		ret.emplace_back(i_path.parent_path(), i_path.filename());
	}

	return ret;
}

} // stego_disk

#endif //HAS_FILESYSTEM_LIBRARY
