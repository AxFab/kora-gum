/*
*      This file is part of the KoraOS project.
*  Copyright (C) 2015  <Fabien Bavent>
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU Affero General Public License as
*  published by the Free Software Foundation, either version 3 of the
*  License, or (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU Affero General Public License for more details.
*
*  You should have received a copy of the GNU Affero General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*   - - - - - - - - - - - - - - -
*/
#ifndef _DIRENT_H
#define _DIRENT_H 1


struct dirent {
    int d_type;
    char d_name[256];
};

void *opendir(const char *path);
struct dirent *readdir(void *dir);
int closedir(void *dir);


#endif  /* _DIRENT_H */
