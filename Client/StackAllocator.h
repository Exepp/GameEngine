#pragma once
#include <stdint.h>

class StackAllocator
{
	typedef uintptr_t Marker;

public:
	explicit StackAllocator(size_t sizeInBytes);
	StackAllocator() = default;
	StackAllocator(const StackAllocator & lref) = delete;
	StackAllocator(StackAllocator&& rref) = delete;
	StackAllocator& operator=(const StackAllocator & lref) = delete;
	StackAllocator& operator=(StackAllocator&& lref) = delete;

	~StackAllocator();

	void init(size_t sizeInBytes);

	void* allocUnaligned(size_t sizeInBytes);
	//  "alignment" must be a power of 2
	void* allocAligned(size_t sizeInBytes, uint8_t alignment);

	// allocate aligned memory and calls default constructor of given type using placement new, to free to this address, call freeToMarkerAligned 
	
	template<class T>
	T* allocConstructed();

	Marker getMarker() const;

	template<class T>
	void freeToUnaligned(T* marker);
	template<class T>
	void freeToAligned(T* marker);

	// sets stack's top marker to bottom
	void clear();
	
	// resets stack size to 0, deletes allocated memory
	void reset();

private:
	Marker topMarker = 0;
	Marker bottomMarker = 0;
	size_t sizeInBytes = 0;
};

template<class T>
inline T * StackAllocator::allocConstructed()
{
	T* ptr = (T*)allocAligned(sizeof(T), alignof(T));
	if (ptr)
		new (ptr) T;
	return ptr;
}

template<class T>
inline void StackAllocator::freeToUnaligned(T * marker)
{
	ASSERT(marker, "Tried to free memory to a nullptr");
	if (marker)
	{
		uintptr_t mark = (uintptr_t)marker;
		ASSERT((mark < topMarker), "Tried to free memory to a marker that lies further than current stack's top marker");

		if (mark < topMarker)
			topMarker = mark;
	}
}

template<class T>
inline void StackAllocator::freeToAligned(T * marker)
{
	ASSERT(marker, "Tried to free memory to a nullptr");
	if (marker)
	{
		uintptr_t mark = (uintptr_t)marker;
		uint8_t adjustment = *((uint8_t*)(mark - 1));
		freeToMarkerUnaligned(mark - adjustment);
	}
}