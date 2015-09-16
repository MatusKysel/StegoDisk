//
//  StegoErrors.h
//  StegoFS
//
//  Created by Martin Kosdy on 4/11/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#ifndef STEGODISK_UTILS_STEGOERRORS_H_
#define STEGODISK_UTILS_STEGOERRORS_H_

#define EXIT_SUCCESS 0

#define SE_BLOCKID_OOR       (-100)  // blockId out of range
#define SE_OUT_OF_RANGE      (-101)  // offset + size out of range

#define SE_LOAD_FILE         (-201)  // unable to load file

#define SE_OPEN_FILE         (-301)  // unable to open file
#define SE_WRITE_FILE        (-302)  // file write error
#define SE_READ_FILE         (-303)  // file read error


#define SE_NOMEM             (-500)  // memory allocation failed

#define SE_NOFILES           (-600)  // there are no files loaded
#define SE_NOT_ENOUGH_SPACE  (-601)  // capacity is too small (not enough space in carriers)

#define SE_UNINITIALIZED     (-700)  // object was not correctly initialized

#define SE_INVALID_CHECKSUM  (-800)  // invalid checksum

#define	STEGO_NO_ERROR	EXIT_SUCCESS
#define	E_UNKNOW_OPTION	(-1)	// neznamy prepinac
#define E_MISSING_ARG	(-2)	// chyba argument
#define E_INVAL         (-3)	// neplatna hodnota argumentu
#define E_RANGE         (-4)	// neplatny rozsah hodnoty argumentu
#define E_NOMEM         (-5)	// nedostatok pamati
#define E_UNKNOWN       (-6)	// neznama chyba
#define	E_PERM          (-7)	// chybajuce prava
#define E_EXIST         (-8)	// ciel nejestvuje (napr. adresar)
#define E_OPEN_DIR      (-9)	// zlyhanie otvorenie adresara
#define E_FORK          (-10)	// zlyhal fork
#define E_PIPE          (-11)	// zlyhalo vytvorenie pipe
#define E_MAKE_DIR      (-12)	// zlyhanie otvorenie adresara
#define E_GET_CWD       (-13)	// zlyhanie otvorenie adresara
#define E_WAIT          (-14)	// zlyhanie pri cakani na ukoncenie detskeho procesu


#endif // STEGODISK_UTILS_STEGOERRORS_H_
