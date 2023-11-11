#pragma once
#include <cstdint>
#include <string>
#include <cassert>
#include <iostream>
#include <unordered_map>
#define LibDLVoiceCodecBegin namespace libdlvcodec {
#define LibDLVoiceCodecEnd }
#define LIBDVCND [[nodiscard]]

#define LibDLVoiceCodecThrow(message) throw std::exception((std::string("[In \"") + __FILE__ + "\" Line " + std::to_string(__LINE__) + "] " + (message)).c_str())

LibDLVoiceCodecBegin

using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;
using float32 = float;
using float64 = double;
using byte = unsigned char;
using lpvoid = void*;
using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

class TensorView;
class Tensor;

const std::unordered_map<std::string, size_t> __Dtype {{"int8", 1}, { "int16", 2 }, { "int32", 4 }, { "int64", 8 },
	{ "float8", 1 }, { "float16", 2 }, { "bfloat16", 2 }, { "float32", 4 }, { "float64", 8 }, { "bool", 1 } };

template<typename T>
class BaseAllocator
{
public:
	BaseAllocator() = default;
	virtual ~BaseAllocator() = default;
	virtual T* allocate(size_t size)
	{
		return new T[size];
	}
	virtual void destroy(const T* ptr)
	{
		delete ptr;
	}
};

template<typename Type, typename Allocator = BaseAllocator<Type>>
class MResource
{
public:
	using reference = Type&;
	using rvalue = Type&&;
	using ptr_t = Type*;

	MResource() = default;

	MResource(size_t _Count)
	{
		data_ = allocator_.allocate(_Count * 2);
		size_ = _Count;
		this_ = data_ + _Count;
		end_ = data_ + _Count * 2;
	}

	MResource(size_t _Count, Type _Value)
	{
		data_ = allocator_.allocate(_Count * 2);
		size_ = _Count;
		this_ = data_ + _Count;
		end_ = data_ + _Count * 2;
		auto _ptr = data_;
		const auto _end = data_ + size_;
		while (_ptr != _end)
		{
			*_ptr = _Value;
			++_ptr;
		}
	}

	MResource(ptr_t _Ptr, size_t _Size)
	{
		data_ = _Ptr;
		size_ = _Size;
		this_ = data_ + _Size;
		end_ = data_ + _Size;
	}

	MResource(const MResource& _Left)
	{
		size_ = _Left.size_;
		data_ = allocator_.allocate(_Left.capacity());
		this_ = data_ + size_;
		end_ = data_ + _Left.capacity();
		auto _ptr = data_, _ptrl = _Left.data_;
		const auto _end = data_ + size_;
		while (_ptr != _end)
		{
			*_ptr = *_ptrl;
			++_ptr;
			++_ptrl;
		}
	}

	MResource(MResource&& _Right) noexcept
	{
		size_ = _Right.size_;
		data_ = _Right.data_;
		this_ = _Right.this_;
		end_ = _Right.end_;
		_Right.size_ = 0ull;
		_Right.data_ = nullptr;
	}

	~MResource()
	{
		if (data_)
		{
			allocator_.destroy(data_);
			data_ = nullptr;
		}
	}

	LIBDVCND ptr_t data() const
	{
		return data_;
	}

	LIBDVCND ptr_t begin() const
	{
		return data_;
	}

	LIBDVCND ptr_t end() const
	{
		return this_;
	}

	ptr_t release()
	{
		const ptr_t _pdata = data_;
		data_ = nullptr;
		return _pdata;
	}

	reference operator[](size_t _Index) const
	{
		assert(_Index < size_);
		return *(data_ + _Index);
	}

	template<typename __Ty>
	reference at(size_t _Index) const
	{
		assert(_Index * sizeof(__Ty) < size_);
		return *((__Ty*)data_ + _Index);
	}

	reference at(size_t _Index) const
	{
		assert(_Index < size_);
		return *(data_ + _Index);
	}

	LIBDVCND size_t size() const
	{
		return size_;
	}

	LIBDVCND size_t capacity() const
	{
		return end_ - data_;
	}

	MResource& operator=(const MResource& _Left)
	{
		if (&_Left == this)
			return *this;
		size_ = _Left.size_;
		data_ = allocator_.allocate(_Left.capacity());
		this_ = data_ + size_;
		end_ = data_ + _Left.capacity();
		auto _ptr = data_, _ptrl = _Left.data_;
		const auto _end = data_ + size_;
		while (_ptr != _end)
		{
			*_ptr = *_ptrl;
			++_ptr;
			++_ptrl;
		}
		return *this;
	}

	MResource& operator=(MResource&& _Right) noexcept
	{
		size_ = _Right.size_;
		data_ = _Right.data_;
		this_ = _Right.this_;
		end_ = _Right.end_;
		_Right.size_ = 0ull;
		_Right.data_ = nullptr;
		return *this;
	}
protected:
	ptr_t data_ = nullptr;
	ptr_t this_ = nullptr;
	ptr_t end_ = nullptr;
	size_t size_ = 0ull;
	Allocator allocator_;
};

template<typename T>
std::ostream& operator<<(std::ostream& _Stream, const MResource<T>& _Data)
{
	_Stream << '[';
	for (const auto& i : _Data)
		_Stream << i << ", ";
	_Stream << "]\n";
	return _Stream;
}

class FileWrapper
{
public:
	FileWrapper() = default;
	~FileWrapper()
	{
		if (file_)
			fclose(file_);
		file_ = nullptr;
	}
	FileWrapper(const FileWrapper& _Left) = delete;
	FileWrapper& operator=(const FileWrapper& _Left) = delete;
	FileWrapper(FileWrapper&& _Right) noexcept
	{
		file_ = _Right.file_;
		_Right.file_ = nullptr;
	}
	FileWrapper& operator=(FileWrapper&& _Right) noexcept
	{
		file_ = _Right.file_;
		_Right.file_ = nullptr;
		return *this;
	}
	void open(const std::wstring& _Path, const std::wstring& _Mode)
	{
#ifdef _WIN32
		_wfopen_s(&file_, _Path.c_str(), _Mode.c_str());
#else
		file_ = _wfopen(_Path.c_str(), _Mode.c_str());
#endif
	}
	operator FILE* () const
	{
		return file_;
	}
	LIBDVCND bool enabled() const
	{
		return file_;
	}
private:
	FILE* file_ = nullptr;
};

LibDLVoiceCodecEnd