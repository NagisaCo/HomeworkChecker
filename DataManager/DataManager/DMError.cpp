﻿//
//  DMError.cpp
//  DataManager
//
//  Created by 林思行 on 2021/5/19.
//

#include "DMError.hpp"

namespace DataManager {

    DMError::DMError(DMErrorType error) {
        switch (error) {
        case SUCCESS:
            break;
        case INVALID_EMAIL:
            throw DMException::INVALID_EMAIL();
            break;
        case INVALID_PASSWOOD:
            throw DMException::INVALID_PASSWOOD();
            break;
        case TARGET_EXISTED:
            throw DMException::TARGET_EXISTED();
            break;
        case TARGET_NOT_FOUND:
            throw DMException::TARGET_NOT_FOUND();
            break;
        case DATABASE_OPERATION_ERROR:
            throw DMException::DATABASE_OPERATION_ERROR();
            break;
        case CONNECTION_ERROR:
            throw DMException::CONNECTION_ERROR();
            break;
        case INVALID_ARGUMENT:
            throw DMException::INVALID_ARGUMENT();
            break;
        case OBJECT_NOT_INITED:
            throw DMException::OBJECT_NOT_INITED();
            break;
        default:
            throw DMException::UNKOWN_ERROR();
            break;
        }
    }

}
