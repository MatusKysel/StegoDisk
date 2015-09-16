//
//  config.h
//  StegoDisk
//
//  Created by Martin Kosdy on 4/15/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

/**
 *    @file config.h
 *    @brief reads/writes files using sftp connection
 *
 *    @author Matus Kysel
 *
 *    @date 30.10.2014
 */

#ifndef STEGODISK_UTILS_CONFIG_H_
#define STEGODISK_UTILS_CONFIG_H_

#define SFS_KEY_HASH_LENGTH        32
#define SFS_BLOCK_SIZE             4*1024

#define SFS_STORAGE_HASH_LENGTH    32

#define SFS_LOGGER_CONFIGFILE      "logger.xml"

#endif // STEGODISK_UTILS_CONFIG_H_
