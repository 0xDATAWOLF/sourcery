#ifndef SOURCERY_SMEMORY_H
#define SOURCERY_SMEMORY_H
#include "primitives.h"
#include <iostream>

/**
 * smemory.h API by Christopher N. DeJong, June 2022, @Github 0xDATAWOLF
 * This is a custom memory allocator used for small-to-medium applications that require
 * fine grained control over its memory allocations. Rather than relying on the C standard
 * library and the STL to handle memory allocations, smemory handles it in way that is both
 * transparent and extensible. As a general allocator, smemory may not be quite as performant
 * as malloc or free but should provide ample performance benefits when memory reclamation
 * and allocations are performed selectively. Additionally, fast-path allocations allow for
 * snappy allocations when accounted for in situations that require many successive allocations.
 * Bulk deallocations can also be achieved by reserving private sectors of memory used for
 * these successive allocations and then marked for reclamation without explicitly invoking
 * free on every allocation made in that region.
 * 
 * Documentation & Details:
 * 
 * JOURNAL_DESCRIPTOR
 * 		Describes a collection of one of more operating system pages that are committed
 * 		and reserved to the application. Journal descriptors are reserved at the head
 * 		of the first page. The heap allocation begins sizeof(JOURNAL_DESCRIPTOR)-bytes
 * 		ahead of the journal descriptor pointer. Journal descriptors contain a set of
 * 		flags that determine their useage. Journals maintain their commit size. As
 * 		allocations are made to journals, both the commit and offset are bumped by
 * 		n-bytes. As memory is freed, only the commit is reduced. This will leave holes
 * 		within the journal, but once a journal reaches zero commit and isn't flagged as
 * 		JDESC_NORECLAIM, it can be given back to the operating system and removed from the
 * 		journal lookup table.
 * 
 * 		JDESC_SHARED:0x0000'0001
 * 			A shared journal will be accessible by the journal lookup table for any
 * 			allocations that don't care which journal they are assigned to. All journals
 * 			dynamically created are given a shared state and non-shared journals must be
 * 			manually created by the user.
 * 		JDESC_NORECLAIM:0x0000'0002
 * 			A journal that is marked as no-reclaim will not be allowed to be given back
 * 			to operating system. The user must manually untoggle the flag in order for
 * 			it to be reclaimed.
 * 		JDESC_FORCERECLAIM:0x0000'00004
 * 			A journal marked as forced-reclaim will be reclaimed by the operating system
 * 			regardless of its commit size. This means any data that resides within will
 * 			be considered deallocated.
 * 
 * ALLOC_DESCRIPTOR
 * 		Describes an allocation made within a journal. Every heap allocation reserves
 * 		an additional 8-bytes of information that describes the total commit of the
 * 		allocation (including the ALLOC_DESCRIPTOR) and the offset relative to the
 * 		journal it resides within.
 * 
 * Journal Lookup Table
 * 		The journal lookup table is a block of memory reserved at initialization time.
 * 		Each entry is a single pointer to a journal descriptor. Every time a journal
 * 		is created, it is stored within the journal lookup table.
 * 
 * smemory::init()
 * 		Performs smemory::_get(), forcing smemory to initialize itself by calling its
 * 		constructor if it wasn't already called. It is recommended to call this at
 * 		application startup to prevent this from being invoked on the first allocation.
 * 
 * smemory::page_size()
 * 		Returns the size of the operating system's memory page size.
 * 
 * smemory::free(void*)
 * 		Frees a region of memory that is managed by smemory; not to be confused with the
 * 		C standard library's free(). They are not compatible with each other.
 * 
 * smemory::alloc(size_t n)
 * 		Allocates a region of memory of at least n-bytes. The actual size of the allocation
 * 		is determined by the requirements to maintain the state of the allocation using
 * 		ALLOC_DESCRIPTOR.
 */

/**
 * The core memory management utility for sourcery.
 */
class smemory
{

	public:

		static void 	init();
		static void 	free(void* addr);
		static void* 	alloc(size_t n);
		static size_t	page_size();

	protected:
		static smemory& _get();

	protected:
		static inline void* 	_ALLOCA_BASE_ADDRESS = (void*)TERABYTES(2); // Virtual memory starting address.
		static inline u32 		_LUPTBL_PAGES = 16; // Number of pages created to construct the page lookup table.

	protected:	
		smemory();
		~smemory() = default;
		
		void* 	_valloc(size_t pages);
		void* 	_get_available_page(size_t allocation_size);

	protected:
		size_t 		_page_size;
		void*		_page_offset_address;
		void*		_page_lookup_table;
		u32			_page_luptbl_ptr_count;
		b32 		_is_initialized;

		struct _page_descriptor
		{
			u32 commit;
			u32 offset;
			u32 ncontigs;
			u32 flags;
		};

		struct _alloc_descriptor
		{
			u32 commit;
			u32 offset;
		};

};

#if (defined(WIN32) || defined(_WIN32))
#include <windows.h>

/** Constructs and initializes smemory. */
smemory::smemory()
{

	// Get the page size.
	SYSTEM_INFO _sys_info = {};
	GetSystemInfo(&_sys_info);
	this->_page_size = (size_t)_sys_info.dwPageSize;

	// Create the page lookup table.
	LPVOID _luptbl = _valloc(_LUPTBL_PAGES);
	_page_lookup_table = _luptbl;

	// Create the first page on the heap.
	LPVOID _heap = _valloc(1);
	_page_descriptor* _pdesc = (_page_descriptor*)_heap;
	_pdesc->commit = 0;
	_pdesc->ncontigs = 1;

	*(void**)(_page_lookup_table) = _heap;

}

/** Initializes smemory rather than initializing on first invocation. */
void smemory::init()
{
	smemory& _smem = smemory::_get(); // Forces smemory to construct on first run.
	return;
}

/** Gets the SMemory instance. */
smemory& smemory::_get()
{
	static smemory _smem = {};
	return _smem;
}

/** Performs a virtual allocation operation to the nearest page boundary that fits the request. */
void* smemory::_valloc(size_t pages)
{
	LPVOID _allocation_offset = (LPVOID)(((u64)_ALLOCA_BASE_ADDRESS) + ((u64)_page_offset_address));
	SIZE_T _allocation_size = (SIZE_T)(pages * _page_size);
	LPVOID _allocation_ptr = VirtualAlloc(_allocation_offset, _allocation_size, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
	_page_offset_address = (void*)((u64)_page_offset_address + (u64)_allocation_size);
	_page_luptbl_ptr_count++;
	return (void*)_allocation_ptr;
}

/** Returns a pointer to the first available page that will fit the requested allocation size. */
void* smemory::_get_available_page(size_t allocation_size)
{

	_page_descriptor* _available_page = nullptr;
	for (u32 i = 0; i < _page_luptbl_ptr_count; ++i)
	{
		// This will invoke fear in the weak of heart.
		_page_descriptor* _cpage = (_page_descriptor*)(*((void**)(_page_lookup_table)+i));

		/**
		 * Each page is monotonic, so we don't want to use the commit of the page to calulcate
		 * the available room. As memory is freed within a page, it will reduce the commit, but will
		 * not move the offset. The available space is the max subtracted by both the offset and
		 * the size of the page descriptor.
		 */
		size_t _cpage_max = _cpage->ncontigs * _page_size; // ncontigs is how many pages are contiguously adjacent to this one.
		size_t _cpage_available = _cpage_max - _cpage->offset - sizeof(_page_descriptor);
		if (allocation_size < _cpage_available)
		{
			_available_page = _cpage;
			break;
		}
	}

	/**
	 * If we could not find a page descriptor that contained enough room to store
	 * the requested allocation, then we will need to create one that can.
	 */
	if (_available_page == nullptr)
	{
		size_t _pn = (allocation_size / _page_size) + 1;
		_available_page = (_page_descriptor*)_valloc(_pn);
	}

	return (void*)_available_page;

}

/** Returns the size of a page. */


/** Allocates n-bytes on the heap. */
void* smemory::alloc(size_t bytes)
{
	smemory& _smem = smemory::_get();

	/**
	 * Each allocation contains it's allocation size and offset relative to the page head.
	 * The lower 32-bits of the allocation description is the size, and the upper 32-bits
	 * of the allocation description contain the relative offset, in bytes, to page head.
	 */
	size_t _n = (bytes + (sizeof(_alloc_descriptor)));
	_n += (_n % 8); // Maintain alignment.

	/**
	 * The _page_descriptor pointer points to the starting address of the page and
	 * details the commit and offset of that page. We can calculate the starting address
	 * of _pptr by adding the _page_descriptor size plus the offset of all allocations.
	 */
	_page_descriptor* _pdescriptor = (_page_descriptor*)_smem._get_available_page(_n);
	void* _pptr = (u8*)_pdescriptor + (sizeof(_page_descriptor) + _pdescriptor->offset);
	_pdescriptor->offset += (u32)_n;
	_pdescriptor->commit += (u32)_n;

	/**
	 * The address we calculated from the page descriptor is actually the starting
	 * address for the allocation descriptor. We will need to fill that out.
	 */
	_alloc_descriptor* _adescriptor = (_alloc_descriptor*)_pptr;
	_adescriptor->commit = (u32)_n; // bytes + sizeof(_alloc_descriptor)
	_adescriptor->offset = (u32)((u64)_pptr - (u64)_pdescriptor);
	void* _allocation_ptr = (u8*)_pptr + sizeof(_alloc_descriptor);

	return _allocation_ptr;

}

/** Frees an allocation from the heap. */
void smemory::free(void* address)
{

	// Get the proper pointer from the address provided. Convert that
	// into the allocation descriptor to get the allocation descriptor
	// size and then 
	void* _pptr = (void*)((u8*)address - sizeof(_alloc_descriptor));
	_alloc_descriptor* _adescriptor = (_alloc_descriptor*)_pptr;

	if (_adescriptor->commit == 0) return; // Already freed.

	void* _pdptr = (u8*)_pptr - _adescriptor->offset;
	_page_descriptor* _pdescriptor = (_page_descriptor*)_pdptr;

	_pdescriptor->commit -= _adescriptor->commit;

	_adescriptor->commit = 0;
	_adescriptor->offset = 0;

	// TODO: Flag the page descriptor to be reclaimed if the commit is zero
	// if needed.

}

#else

#endif

#endif