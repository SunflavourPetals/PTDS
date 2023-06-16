#pragma once

#ifndef Petal_Header_PTDS_Included
#define Petal_Header_PTDS_Included


#ifdef Petal_DLL_PTDS_DLLExport
#define _Petal_LIB _declspec(dllexport)
#else
#ifdef Petal_LIB_PTDS
#define _Petal_LIB
#else
#define _Petal_LIB _declspec(dllimport)
#endif // Petal_LIB_PTDS
#endif // Petal_DLL_PTDS_DLLExport


#include <unordered_map>

#if defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS) || defined(_CONSOLE)
#if !defined(Petal_Header_TString_Included)
#include <xstring>
namespace Petal { using WString = ::std::wstring; }
#endif
#else
#include <xstring>
// not supported
// namespace Petal { using WString = ::std::u16string; }
namespace Petal { using WString = ::std::wstring; }
#endif

namespace Petal
{
	namespace PTDSBasicType
	{
#if defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS) || defined(_CONSOLE)
		using char_utf16le = wchar_t;
		using str_utf16le = WString;
#else
		//	using char_utf16le = char16_t;
		//	using str_utf16le = ::std::u16string;
		using char_utf16le = unsigned short;
		using str_utf16le = ::std::vector<unsigned short>;
#endif
		using u8 = unsigned char;
		using u16 = unsigned short int;
		using u32 = unsigned long int;
		using u64 = unsigned long long int;
		using i8 = signed char;
		using i16 = signed short int;
		using i32 = signed long int;
		using i64 = signed long long int;
		using f32 = float;
		using f64 = double;
		using bln = bool;
		using cha = char_utf16le;
		using str = str_utf16le;
#if defined(_WIN64)
		using tsize = unsigned long long;
#elif defined(_WIN32)
		using tsize = unsigned long;
#else
		using tsoze = unsigned long long;
#endif
	}
	enum class PTDSBasicTypeEnum
	{
		Unknown = 0,
		u8 = 1,
		u16 = 2,
		u32 = 3,
		u64 = 4,
		i8 = 5,
		i16 = 6,
		i32 = 7,
		i64 = 8,
		f32 = 9,
		f64 = 10,
		cha = 11,
		str = 12,
		bln = 16,
	};
	enum class PTDSFileErrorCode
	{
		Unknown = 0,
		FailedInOpenFile = 1,
		FailedInQueryFileSize = 2,
		OddFileSize = 3,
		FailedInReadFile = 4,
		OddReadBytes = 5,
		NullFile = 6,
		FloatBufferReference = 7,
		RefExceedVailedSize = 64,
		RefBeforeVailedSize = 65,
		RefOffsetTooBig = 66,
		RefOffsetTooSmall = 67,
		NotUTF16LE = 127,
	};
	struct PTDSValueType
	{
		PTDSValueType() : u64(0) {}
		union
		{
			PTDSBasicType::u8 u8;
			PTDSBasicType::i8 i8;
			PTDSBasicType::u16 u16;
			PTDSBasicType::i16 i16;
			PTDSBasicType::u32 u32;
			PTDSBasicType::i32 i32;
			PTDSBasicType::u64 u64;
			PTDSBasicType::i64 i64;
			PTDSBasicType::f32 f32;
			PTDSBasicType::f64 f64;
			PTDSBasicType::cha cha;
			PTDSBasicType::bln bln;
		};
		PTDSBasicType::str str;
	};
	struct PTDSValueSet
	{
		::std::vector<PTDSValueType> v;
		PTDSBasicTypeEnum t{ PTDSBasicTypeEnum::Unknown };
	};
	using PTDO = ::std::unordered_map<PTDSBasicType::str_utf16le, PTDSValueSet>;
	class _Petal_LIB PTDS
	{
	public:
		void LoadPTDS(const PTDSBasicType::char_utf16le* file_name_);
		void LoadPTDSFromOuterBuffer(const PTDSBasicType::char_utf16le* buffer, const PTDSBasicType::tsize& size);
		void LoadPTDSFromBuffer(const PTDSBasicType::char_utf16le* buffer, const PTDSBasicType::tsize& size);
		const PTDO& PTDSObject() const;
		const PTDSValueSet& Entity(const WString& tag) const;
		const PTDSBasicType::i8& ElementI8(const WString& tag, const PTDSBasicType::tsize& index = 0) const;
		const PTDSBasicType::i16& ElementI16(const WString& tag, const PTDSBasicType::tsize& index = 0) const;
		const PTDSBasicType::i32& ElementI32(const WString& tag, const PTDSBasicType::tsize& index = 0) const;
		const PTDSBasicType::i64& ElementI64(const WString& tag, const PTDSBasicType::tsize& index = 0) const;
		const PTDSBasicType::u8& ElementU8(const WString& tag, const PTDSBasicType::tsize& index = 0) const;
		const PTDSBasicType::u16& ElementU16(const WString& tag, const PTDSBasicType::tsize& index = 0) const;
		const PTDSBasicType::u32& ElementU32(const WString& tag, const PTDSBasicType::tsize& index = 0) const;
		const PTDSBasicType::u64& ElementU64(const WString& tag, const PTDSBasicType::tsize& index = 0) const;
		const PTDSBasicType::f32& ElementF32(const WString& tag, const PTDSBasicType::tsize& index = 0) const;
		const PTDSBasicType::f64& ElementF64(const WString& tag, const PTDSBasicType::tsize& index = 0) const;
		const PTDSBasicType::bln& ElementBool(const WString& tag, const PTDSBasicType::tsize& index = 0) const;
		const PTDSBasicType::cha& ElementChar(const WString& tag, const PTDSBasicType::tsize& index = 0) const;
		const PTDSBasicType::str& ElementStr(const WString& tag, const PTDSBasicType::tsize& index = 0) const;
		const PTDSBasicType::str& OriElemStr(const WString& tag, const PTDSBasicType::tsize& index = 0) const;
	public:
		PTDS();
		PTDS(const PTDS&);
		PTDS(PTDS&&) noexcept;
		~PTDS();
		PTDS& operator=(const PTDS&);
	private:
		void* pto;
	};
	enum class PTDSQueryErrorCode
	{
		Unknown = 0,
		Succeed = Unknown,
		CanNotFindTag = 1,
		WrongType = 2,
		OutOfRange = 3,
	};
	class PTDSQueryException
	{
	public:
		PTDSQueryErrorCode error_code{ PTDSQueryErrorCode::Unknown };
		PTDSBasicTypeEnum actual_type{ PTDSBasicTypeEnum::Unknown };
		PTDSBasicType::tsize actual_size{ 0 };
	};
	enum class PTDSErrorCode
	{
		Unknown = 0,
		FailedInOpenFile = 1,
		CommentNotClosed = 2,
		IllegalCharacter = 3,
		CanNotFindTag = 5,
		TagNotClosed = 6,
		TagIsNull = 7,
		TagNotUnique = 8,
		NotNumber = 9,
		CanNotFindEntity = 10,
		QuoteNotClosed = 11,
		TypeNotClosed = 12,
		UnknownType = 14,
		NumberOutOfRange = 15,
		TypeNotUnique = 16,
		WrongType = 17,
		WrongSize = 18,
		BlockNotClosed = 19,
	};
	class PTDSFile;
	class _Petal_LIB PTDSException
	{
	public:
		PTDSException() = default;
		PTDSException(PTDSErrorCode code, const PTDSBasicType::char_utf16le* msg, PTDSBasicType::tsize line_, PTDSBasicType::tsize position_, PTDSBasicType::char_utf16le character_);
		PTDSException(PTDSErrorCode code, const PTDSBasicType::char_utf16le* msg, const PTDSFile& pts);
		PTDSException(const PTDSException&) = default;
		PTDSException(PTDSException&&) = default;
		~PTDSException() = default;
	public:
		PTDSErrorCode error_code{ PTDSErrorCode::Unknown };
		const PTDSBasicType::char_utf16le* message{ L"Unknown" };
		PTDSBasicType::tsize line{ 0 };
		PTDSBasicType::tsize position{ 0 };
		PTDSBasicType::char_utf16le character{ L'0' };
	};
}

#undef _Petal_DLL

#endif // !Petal_Header_PTDS_Included