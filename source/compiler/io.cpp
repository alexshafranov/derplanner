//
// Copyright (c) 2013 Alexander Shafranov shafranov@gmail.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#include <stdio.h>
#include "derplanner/compiler/io.h"

namespace plnnrc
{

Stdio_File_Writer::Stdio_File_Writer(void* file_object)
    : _file_object(file_object)
    , _error(false)
{
}

size_t Stdio_File_Writer::write(const void* data, size_t size)
{
    size_t result = fwrite(data, 1, size, static_cast<FILE*>(_file_object));

    if (size != result)
    {
        _error = true;
    }

    return result;
}

}
