#ifndef SOURCERY_FHANDLE_H
#define SOURCERY_FHANDLE_H

struct file_handle
{
	void* oshandle;
	file_handle(void* oshandle, size_t hsize);
};

file_handle::file_handle(void* oshandle, size_t hsize)
{
	
}

#endif