/*
#ifndef Petal_LIB_PTDS
#define Petal_LIB_PTDS
#endif // !Petal_LIB_PTDS
*/
/*
#ifndef Petal_DLL_PTDS_DLLExport
#define Petal_DLL_PTDS_DLLExport
#endif // !Petal_DLL_PTDS_DLLExport
*/
#include "PTDS.h"

#include <unordered_set>
#include <vector>
#include <string>
#include <stdexcept>
#include <memory>
#include <Windows.h> // Win32 API for read files

namespace Petal
{
	static constexpr PTDSBasicType::char_utf16le keyword_tagopen{ L'[' };
	static constexpr PTDSBasicType::char_utf16le keyword_tagclose{ L']' };
	static constexpr PTDSBasicType::char_utf16le keyword_typeopen{ L'<' };
	static constexpr PTDSBasicType::char_utf16le keyword_typeclose{ L'>' };
	static constexpr PTDSBasicType::char_utf16le keyword_blockopen{ L'{' };
	static constexpr PTDSBasicType::char_utf16le keyword_blockclose{ L'}' };
	static constexpr PTDSBasicType::char_utf16le keyword_quote{ L'\"' };
	static constexpr PTDSBasicType::char_utf16le keyword_separator{ L',' };
	static constexpr PTDSBasicType::char_utf16le keyword_tagconnect{ L':' };
	static constexpr PTDSBasicType::char_utf16le keyword_ex_bracketopen{ L'(' };
	static constexpr PTDSBasicType::char_utf16le keyword_ex_bracketclose{ L')' };
	static constexpr PTDSBasicType::char_utf16le keyword_ex_div{ L'/' };
	static constexpr PTDSBasicType::char_utf16le keyword_ex_mul{ L'*' };
	static constexpr PTDSBasicType::char_utf16le ig_space{ L' ' };
	static constexpr PTDSBasicType::char_utf16le ig_table{ L'\t' };
	static constexpr PTDSBasicType::char_utf16le ig_null{ L'\0' };
	static constexpr PTDSBasicType::char_utf16le ig_cr{ L'\r' };
	static constexpr PTDSBasicType::char_utf16le ig_lf{ L'\n' };
	static constexpr PTDSBasicType::char_utf16le type_post_f{ L'f' };
	static constexpr PTDSBasicType::char_utf16le type_post_F{ L'F' };
	static constexpr PTDSBasicType::char_utf16le type_post_c{ L'c' };
	static constexpr PTDSBasicType::char_utf16le type_post_C{ L'C' };
}
// PTDSFile
namespace Petal
{
	class PTDSFile final
	{
	public:
		bool Open(const PTDSBasicType::char_utf16le* file_name_);
		bool RefOuterBuffer(const PTDSBasicType::char_utf16le* buffer, const PTDSBasicType::tsize& size);
		bool CopyOuterAsBuffer(const PTDSBasicType::char_utf16le* buffer, const PTDSBasicType::tsize& size);
		// 返回索引处的字符的引用, 此函数不会失败, 总会索引到缓冲区内某个值, 但是是否有效需要查看 EndOfFile 的返回值, 当 EndOfFile 时索引到的字符无效
		const PTDSBasicType::char_utf16le& Ref() const;
		// 访问 索引前100至后100位置处的字符, 如果超过此值函数将失败, 返回L'\0', error 为非 Unknown
		// 索引前没有100个字符或后没有100个字符, 但是却被访问到此位置时, 返回L'\0', error 为非 Unknown
		const PTDSBasicType::char_utf16le& Ref(signed int addr, PTDSFileErrorCode& error);
		// 索引到下一个字符, 如果遇到换行符应当停止 Next 操作, 调用 ProcLineFeed 处理换行符
		bool Next();
		// 文件是否结束
		bool EndOfFile() const;
		// 检查当前 index 引用的字符是否可以略过, 如控制字符、空格可略过, 返回 false. true 为不可略过
		bool CheckValid();
		// 处理并跳过换行, 只有在遇到换行符(index 索引到CR或LF时)才能使用, 否则抛出异常
		void ProcLineFeed();
		// 去除 BOM 后文件中 utf-16 字符的数量
		PTDSBasicType::tsize ValidFileSize() const;
		// 当时索引到的字符所在的行数
		PTDSBasicType::tsize Line() const;
		// 当时索引到的字符所在的列数
		PTDSBasicType::tsize Position() const;
		// line 数加一, 通常由 NextValid 调用, 其他函数不应当处理换行符, 调用此函数后 position 归一
		void AddLine(int adt = 1);
		// position 数加一, 通常由 Next 调用, 其他函数不应当直接改变索引值, 只能通过 Next 间接修改
		void AddPosition(int adt = 1);
		// 设置 position 为指定值
		void ResetPosition(int pos = 1);
		// 得到当前索引处字符的地址
		const PTDSBasicType::char_utf16le* CurrRefAddr() const;
		// 得到索引值, 索引与数据在文件中的位置对应
		PTDSBasicType::tsize Index() const;
		PTDSFileErrorCode LastError() const;
	public:
		PTDSFile() = default;
		PTDSFile(const PTDSFile&&) = delete;
		PTDSFile(PTDSFile&&) = delete;
		~PTDSFile();
		PTDSFile& operator=(const PTDSFile&) = delete;
	protected:
		PTDSBasicType::char_utf16le* buffer_1{ nullptr };
		PTDSBasicType::char_utf16le* buffer_2{ nullptr };
		const PTDSBasicType::char_utf16le* buffer_ref{ nullptr };
		PTDSBasicType::tsize valid_size{ 0 };
		PTDSBasicType::tsize base_index{ 0 };
		PTDSBasicType::tsize curr_index{ 0 };
		PTDSBasicType::tsize line{ 1 };
		PTDSBasicType::tsize position{ 1 };
		PTDSFileErrorCode error_code{ PTDSFileErrorCode::Unknown };
		HANDLE file_handle{ nullptr };
		bool buffer_flipped{ false };
		bool load_from_file{ true };
	};
}
namespace Petal
{
	namespace PTDSTools
	{
		// 过滤注释函数, 使用时 index 应索引至注释起始标志之后紧接的字符, 如"  ["abc..."]"中,
		// 使用以下四个函数时 this 的 index 应索引至'a'位置, 过滤注释起始标志由调用方完成, 减少重复检查起始标志以优化效率
		void FilComment(PTDSFile& pts, PTDSBasicType::char_utf16le closebegin = keyword_quote, PTDSBasicType::char_utf16le closeend = keyword_tagclose)
		{
			PTDSFileErrorCode error{ PTDSFileErrorCode::Unknown };
			PTDSBasicType::tsize line = pts.Line();
			PTDSBasicType::tsize position = pts.Position();
			while (!pts.EndOfFile())
			{
				if (pts.Ref() == closebegin)
				{
					if (pts.Ref(1, error) == closeend && error == PTDSFileErrorCode::Unknown)
					{
						pts.Next();
						pts.Next();
						return;
					}
				}
				switch (pts.Ref())
				{
				case ig_null:
				case ig_space:
				case ig_table:
					break;
				case ig_cr:
				case ig_lf:
					pts.ProcLineFeed();
					continue;
					break;
				default:
					break;
				}
				pts.Next();
			}
			throw PTDSException(PTDSErrorCode::CommentNotClosed, L"CommentNotClosed", line, position, L'\0');
		}
		void FilCommentEx(PTDSFile& pts)
		{
			FilComment(pts, keyword_quote, keyword_ex_bracketclose);
		}
		void FilCommentC(PTDSFile& pts)
		{
			FilComment(pts, keyword_ex_mul, keyword_ex_div);
		}
		void FilCommentCpp(PTDSFile& pts)
		{
			PTDSFileErrorCode error{ PTDSFileErrorCode::Unknown };
			PTDSBasicType::tsize line = pts.Line();
			PTDSBasicType::tsize position = pts.Position();
			while (!pts.EndOfFile())
			{
				switch (pts.Ref())
				{
				case ig_cr:
					if (pts.Ref(1, error) == ig_lf && error == PTDSFileErrorCode::Unknown)
					{
						pts.Next();
					}
					pts.Next();
					pts.AddLine();
					return;
					break;
				case ig_lf:
					pts.Next();
					pts.AddLine();
					return;
					break;
				default:
					break;
				}
				pts.Next();
			}
		}
		// 转到下一个有效字符, 在 pts 里控制字符和空格等是无关紧要的, 使用此函数跳过无关字符和注释段, 此函数会自动记录 line 和 position
		void NextValid(PTDSFile& pts)
		{
			PTDSFileErrorCode error{ PTDSFileErrorCode::Unknown };
			while (!pts.EndOfFile())
			{
				switch (pts.Ref())
				{
				case ig_cr:
				case ig_lf:
					pts.ProcLineFeed();
					continue;
					break;
				case keyword_tagopen:
					if (pts.Ref(1, error) == keyword_quote && error == PTDSFileErrorCode::Unknown)
					{
						pts.Next();
						pts.Next();
						// comment
						FilComment(pts);
						continue;
					}
					else { return; }
					break;
				case keyword_ex_bracketopen:
					if (pts.Ref(1, error) == keyword_quote && error == PTDSFileErrorCode::Unknown)
					{
						pts.Next();
						pts.Next();
						// comment ex
						FilCommentEx(pts);
						continue;
					}
					else { return; }
					break;
				case keyword_ex_div:
					if (pts.Ref(1, error) == keyword_ex_mul && error == PTDSFileErrorCode::Unknown)
					{
						pts.Next();
						pts.Next();
						// comment c
						FilCommentC(pts);
						continue;
					}
					else if (pts.Ref(1, error) == keyword_ex_div && error == PTDSFileErrorCode::Unknown)
					{
						pts.Next();
						pts.Next();
						// comment cpp
						FilCommentCpp(pts);
						continue;
					}
					else { return; }
					break;
				default:
					if (pts.CheckValid() == false)
					{
						break;
					}
					else
					{
						return;
					}
					break;
				}
				pts.Next();
			}
		}
	}
	namespace PTDSFileTools
	{
		void SpecifyAnotherBuffer(
			PTDSBasicType::char_utf16le* const& b1,
			PTDSBasicType::char_utf16le* const& b2,
			const PTDSBasicType::char_utf16le* const& b_ref,
			PTDSBasicType::char_utf16le** dest,
			PTDSFileErrorCode& error_code)
		{
			if (b_ref == b1)
			{
				*dest = b2;
			}
			else if (b_ref == b2)
			{
				*dest = b1;
			}
			else
			{
				error_code = PTDSFileErrorCode::FloatBufferReference;
			}
		}
	}
	static constexpr PTDSBasicType::tsize buffer_size{ 512 };
	PTDSFile::~PTDSFile()
	{
		if (buffer_1 != nullptr) delete[] buffer_1;
		buffer_1 = nullptr;
		if (buffer_2 != nullptr) delete[] buffer_2;
		buffer_2 = nullptr;
		buffer_ref = nullptr;
		if (file_handle != INVALID_HANDLE_VALUE && file_handle != nullptr) ::CloseHandle(file_handle);
		file_handle = nullptr;
	}
	bool PTDSFile::Open(const PTDSBasicType::char_utf16le* file_name_)
	{
		error_code = PTDSFileErrorCode::Unknown;
		if (file_handle != INVALID_HANDLE_VALUE && file_handle != 0)
		{
			::CloseHandle(file_handle);
			file_handle = nullptr;
		}
		file_handle = ::CreateFileW(file_name_, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (file_handle == INVALID_HANDLE_VALUE || file_handle == 0)
		{
			error_code = PTDSFileErrorCode::FailedInOpenFile;
			return false;
		}
		LARGE_INTEGER li{};
		if (::GetFileSizeEx(file_handle, &li) == FALSE)
		{
			error_code = PTDSFileErrorCode::FailedInQueryFileSize;
			if (file_handle != INVALID_HANDLE_VALUE && file_handle != 0)
			{
				::CloseHandle(file_handle);
				file_handle = nullptr;
			}
			return false;
		}
		if (li.QuadPart % sizeof(PTDSBasicType::char_utf16le) != 0)
		{
			error_code = PTDSFileErrorCode::OddFileSize;
			if (file_handle != INVALID_HANDLE_VALUE && file_handle != 0)
			{
				::CloseHandle(file_handle);
				file_handle = nullptr;
			}
			return false;
		}
		if (li.QuadPart <= 0)
		{
			error_code = PTDSFileErrorCode::NullFile;
			if (file_handle != INVALID_HANDLE_VALUE && file_handle != 0)
			{
				::CloseHandle(file_handle);
				file_handle = nullptr;
			}
			return false;
		}
		valid_size = static_cast<decltype(valid_size)>(li.QuadPart / sizeof(PTDSBasicType::char_utf16le));
		{
			DWORD target_read_num = static_cast<DWORD>(2);
			DWORD read_num{ 0 };
			unsigned char buffer[4]{};
			if (::ReadFile(file_handle, buffer, target_read_num, &read_num, nullptr) == FALSE)
			{
				error_code = PTDSFileErrorCode::FailedInReadFile;
				if (file_handle != INVALID_HANDLE_VALUE && file_handle != 0)
				{
					::CloseHandle(file_handle);
					file_handle = nullptr;
				}
				return false;
			}
			if (read_num < 2)
			{
				error_code = PTDSFileErrorCode::OddReadBytes;
				if (file_handle != INVALID_HANDLE_VALUE && file_handle != 0)
				{
					::CloseHandle(file_handle);
					file_handle = nullptr;
				}
				return false;
			}
			const unsigned char UTF16LE[4]{ 0xFFu, 0xFEu, 0x00u, 0x00u };
			if (buffer[0] != UTF16LE[0] || buffer[1] != UTF16LE[1])
			{
				error_code = PTDSFileErrorCode::NotUTF16LE;
				if (file_handle != INVALID_HANDLE_VALUE && file_handle != 0)
				{
					::CloseHandle(file_handle);
					file_handle = nullptr;
				}
				return false;
			}
			valid_size -= 1;
		}

		if (buffer_1 != nullptr) delete[] buffer_1;
		buffer_1 = nullptr;
		if (buffer_2 != nullptr) delete[] buffer_2;
		buffer_2 = nullptr;

		buffer_1 = new PTDSBasicType::char_utf16le[buffer_size];
		buffer_2 = new PTDSBasicType::char_utf16le[buffer_size];

		DWORD target_read_num = static_cast<DWORD>(li.QuadPart);
		DWORD read_num{ 0 };

		if (target_read_num > buffer_size * sizeof(PTDSBasicType::char_utf16le))
		{
			target_read_num = static_cast<DWORD>(buffer_size * sizeof(PTDSBasicType::char_utf16le));
		}

		if (::ReadFile(file_handle, buffer_1, target_read_num, &read_num, nullptr) == FALSE)
		{
			if (buffer_1 != nullptr) delete[] buffer_1;
			if (buffer_2 != nullptr) delete[] buffer_2;
			error_code = PTDSFileErrorCode::FailedInReadFile;
			return false;
		}
		if (read_num % sizeof(PTDSBasicType::char_utf16le) != 0)
		{
			if (buffer_1 != nullptr) delete[] buffer_1;
			if (buffer_2 != nullptr) delete[] buffer_2;
			error_code = PTDSFileErrorCode::OddReadBytes;
			return false;
		}
		load_from_file = true;
		base_index = 0;
		curr_index = 0;
		buffer_ref = buffer_1;
		line = 1;
		position = 1;
		buffer_flipped = false;
		return true;
	}
	bool PTDSFile::RefOuterBuffer(const PTDSBasicType::char_utf16le* buffer, const PTDSBasicType::tsize& size)
	{
		if (buffer == nullptr)
		{
			error_code = PTDSFileErrorCode::FailedInOpenFile;
			return false;
		}
		if (size < 1)
		{
			error_code = PTDSFileErrorCode::NullFile;
			return false;
		}
		this->~PTDSFile();
		load_from_file = false;
		error_code = PTDSFileErrorCode::Unknown;
		valid_size = size;
		base_index = 0;
		curr_index = 0;
		line = 1;
		position = 1;
		buffer_flipped = false;
		buffer_ref = buffer;

		const unsigned char* bom_ptr{ reinterpret_cast<const unsigned char*>(buffer_ref) };
		const unsigned char UTF16LE[4]{ 0xFFu, 0xFEu, 0x00u, 0x00u };
		if (bom_ptr[0] == UTF16LE[0] && bom_ptr[1] == UTF16LE[1])
		{
			buffer_ref += 1;
			valid_size -= 1;
			if (valid_size < 1)
			{
				error_code = PTDSFileErrorCode::NullFile;
				return false;
			}
		}
		return true;
	}
	bool PTDSFile::CopyOuterAsBuffer(const PTDSBasicType::char_utf16le* buffer, const PTDSBasicType::tsize& size)
	{
		if (buffer == nullptr)
		{
			error_code = PTDSFileErrorCode::FailedInOpenFile;
			return false;
		}
		if (size < 1)
		{
			error_code = PTDSFileErrorCode::NullFile;
			return false;
		}
		this->~PTDSFile();
		load_from_file = false;
		error_code = PTDSFileErrorCode::Unknown;
		valid_size = size;
		base_index = 0;
		curr_index = 0;
		line = 1;
		position = 1;
		buffer_flipped = false;

		constexpr PTDSBasicType::tsize type_size{ sizeof(PTDSBasicType::char_utf16le) };
		buffer_1 = new PTDSBasicType::char_utf16le[size + 1];
		const PTDSBasicType::tsize buffer_size{ size* type_size };
		memcpy_s(buffer_1, buffer_size, buffer, buffer_size);
		buffer_1[size] = L'\0';
		buffer_ref = buffer_1;

		const unsigned char* bom_ptr{ reinterpret_cast<const unsigned char*>(buffer_ref) };
		const unsigned char UTF16LE[4]{ 0xFFu, 0xFEu, 0x00u, 0x00u };
		if (bom_ptr[0] == UTF16LE[0] && bom_ptr[1] == UTF16LE[1])
		{
			buffer_ref += 1;
			valid_size -= 1;
			if (valid_size < 1)
			{
				error_code = PTDSFileErrorCode::NullFile;
				return false;
			}
		}
		return true;
	}
	const PTDSBasicType::char_utf16le& PTDSFile::Ref() const
	{
		return buffer_ref[curr_index];
	}
	const PTDSBasicType::char_utf16le& PTDSFile::Ref(signed int addr, PTDSFileErrorCode& error)
	{
		static constexpr PTDSBasicType::char_utf16le null_char{ L'\0' };
		if (load_from_file == false)
		{
			if ((static_cast<PTDSBasicType::i64>(curr_index) + addr) < 0)
			{
				error = PTDSFileErrorCode::RefOffsetTooSmall;
				return null_char;
			}
			const PTDSBasicType::tsize target_index{ curr_index + addr };
			if (target_index >= valid_size)
			{
				error = PTDSFileErrorCode::RefOffsetTooBig;
				return null_char;
			}
			return buffer_ref[target_index];
		}
		static constexpr long long buffer_size_ll{ static_cast<long long>(buffer_size) };
		static constexpr int max_valid_offset{ +100 };
		static constexpr int min_valid_offset{ -100 };
		if (addr > max_valid_offset)
		{
			error = PTDSFileErrorCode::RefOffsetTooBig;
			return null_char;
		}
		else if (addr < min_valid_offset)
		{
			error = PTDSFileErrorCode::RefOffsetTooSmall;
			return null_char;
		}
		error = PTDSFileErrorCode::Unknown;
		const long long base_index_ll{ static_cast<long long>(base_index) };
		const long long valid_size_ll{ static_cast<long long>(valid_size) };
		signed long long target_index{ static_cast<signed long long>(curr_index) + addr };
		PTDSBasicType::char_utf16le* buffer_next{ nullptr };

		if (target_index >= buffer_size_ll)
		{
			if (target_index + base_index_ll >= valid_size_ll)
			{
				error = PTDSFileErrorCode::RefExceedVailedSize;
				return null_char;
			}
			PTDSFileTools::SpecifyAnotherBuffer(buffer_1, buffer_2, buffer_ref, &buffer_next, error);
			if (buffer_next == nullptr)
			{
				return null_char;
			}
			target_index -= buffer_size_ll;
			return buffer_next[target_index];
		}
		if (target_index < 0)
		{
			if (base_index_ll + target_index < 0)
			{
				error = PTDSFileErrorCode::RefBeforeVailedSize;
				return null_char;
			}
			PTDSFileTools::SpecifyAnotherBuffer(buffer_1, buffer_2, buffer_ref, &buffer_next, error_code);
			if (buffer_next == nullptr)
			{
				return null_char;
			}
			target_index += buffer_size_ll;
			return buffer_next[target_index];
		}
		if (target_index + base_index_ll >= valid_size_ll)
		{
			error = PTDSFileErrorCode::RefExceedVailedSize;
			return null_char;
		}
		return buffer_ref[target_index];
	}
	bool PTDSFile::Next()
	{
		if (this->EndOfFile())
		{
			return false;
		}
		++curr_index;
		++position;
		if (load_from_file == false)
		{
			return true;
		}
		if ((buffer_flipped == false) && (curr_index >= buffer_size / 2) && (valid_size > base_index + buffer_size))
		{
			buffer_flipped = true;
			PTDSBasicType::char_utf16le* buffer_next{ nullptr };
			if (buffer_ref == buffer_1)
			{
				buffer_next = buffer_2;
			}
			else if (buffer_ref == buffer_2)
			{
				buffer_next = buffer_1;
			}
			else
			{
				error_code = PTDSFileErrorCode::FloatBufferReference;
				return false;
			}
			constexpr DWORD target_read_num{ static_cast<DWORD>(buffer_size * sizeof(PTDSBasicType::char_utf16le)) };
			DWORD read_num{ 0 };
			if (::ReadFile(file_handle, buffer_next, target_read_num, &read_num, nullptr) == FALSE)
			{
				error_code = PTDSFileErrorCode::FailedInReadFile;
				return false;
			}
		}
		if (curr_index >= buffer_size)
		{
			buffer_flipped = false;
			if (buffer_ref == buffer_1)
			{
				buffer_ref = buffer_2;
			}
			else if (buffer_ref == buffer_2)
			{
				buffer_ref = buffer_1;
			}
			base_index += buffer_size;
			curr_index = 0;
		}
		return true;
	}
	bool PTDSFile::EndOfFile() const
	{
		return base_index + curr_index >= valid_size;
	}
	PTDSBasicType::tsize PTDSFile::ValidFileSize() const
	{
		return valid_size;
	}
	PTDSBasicType::tsize PTDSFile::Line() const
	{
		return line;
	}
	PTDSBasicType::tsize PTDSFile::Position() const
	{
		return position;
	}
	void PTDSFile::AddLine(int adt)
	{
		line += adt;
		this->ResetPosition();
	}
	void PTDSFile::AddPosition(int adt)
	{
		position += adt;
	}
	void PTDSFile::ResetPosition(int pos)
	{
		position = pos;
	}
	const PTDSBasicType::char_utf16le* PTDSFile::CurrRefAddr() const
	{
		return &(buffer_ref[curr_index]);
	}
	PTDSBasicType::tsize PTDSFile::Index() const
	{
		return base_index + curr_index;
	}
	PTDSFileErrorCode PTDSFile::LastError() const
	{
		return error_code;
	}
	void PTDSFile::ProcLineFeed()
	{
		PTDSFile& pts{ *this };
		PTDSFileErrorCode error{ PTDSFileErrorCode::Unknown };
		switch (pts.Ref())
		{
		case ig_cr:
			if (pts.Ref(1, error) == ig_lf && error == PTDSFileErrorCode::Unknown)
			{
				pts.Next();
			}
			pts.Next();
			pts.AddLine();
			return;
			break;
		case ig_lf:
			pts.Next();
			pts.AddLine();
			return;
			break;
		default:
			throw PTDSException(PTDSErrorCode::IllegalCharacter, L"Process Line Feed when not catch Line Feed", pts);
			break;
		}
	}
	bool PTDSFile::CheckValid()
	{
		auto& pts{ *this };
		if (pts.Ref() >= L'\x0000' && pts.Ref() <= L'\x001f')
		{
			return false;
		}
		switch (pts.Ref())
		{
		case L'\x7f':
		case ig_null:
		case ig_space:
		case ig_table:
		case ig_cr:
		case ig_lf:
			return false;
			break;
		default:
			break;
		}
		return true;
	}
}
// PTDSException
namespace Petal
{
	PTDSException::PTDSException(PTDSErrorCode code, const PTDSBasicType::char_utf16le* msg, PTDSBasicType::tsize line_, PTDSBasicType::tsize position_, PTDSBasicType::char_utf16le character_) :
		error_code(code),
		message(msg),
		line(line_),
		position(position_),
		character(character_) { }
	PTDSException::PTDSException(PTDSErrorCode code, const PTDSBasicType::char_utf16le* msg, const PTDSFile& pts) :
		error_code(code),
		message(msg),
		line(pts.Line()),
		position(pts.Position()),
		character(pts.Ref()) { }
}

namespace Petal
{
#ifdef emsg
#undef emsg
#endif // emsg
#define emsg(x) constexpr PTDSBasicType::char_utf16le x[]
	namespace PTDSErrorMsg
	{

		emsg(FailedInOpenFile) = L"Failed in open file, the line msg is PTDSFileErrorCode";
		emsg(IllegalCharacter) = L"Illegal character";
		emsg(CanNotFindTag) = L"Can not find tag";
		emsg(TagNotClosed) = L"Square brackets are not closed";
		emsg(TagIsNull) = L"Tag is null";
		emsg(TagNotUnique) = L"Tag not unique";
		emsg(NotNumber) = L"Not number";
		emsg(CanNotFindEntity) = L"Can not find entity";
		emsg(QuoteNotClosed) = L"Quotes not closed";
		emsg(TypeNotClosed) = L"Angle brackets are not closed";
		emsg(UnknownType) = L"Unknown type";
		emsg(NumberOutOfRange) = L"The value of number out of range";
		emsg(TypeNotUnique) = L"Type not unique";
		emsg(WrongType) = L"Wrong type";
		emsg(WrongSize) = L"Wrong size";
		emsg(BlockNotClosed) = L"Braces are not closed";
	}
#undef emsg
	namespace PTDSTools
	{
		using TSP = ::std::vector<PTDSBasicType::str_utf16le>;
		using Tag = PTDSBasicType::str_utf16le;
		using TagSet = ::std::unordered_set<Tag>;
		struct AttributeRecord
		{
			PTDSBasicTypeEnum type{ PTDSBasicTypeEnum::Unknown };
			PTDSBasicType::str_utf16le sign; // + -
			PTDSBasicType::str_utf16le format; // 0X 0x 0B 0b
			PTDSBasicType::str_utf16le value;
			PTDSBasicType::tsize line{ 0 };
			PTDSBasicType::tsize position{ 0 };
			bool hex_open{ false };
			bool bin_open{ false };
			bool float_str{ false };
			bool positive{ true };
		};
		struct AttributeRecordSet
		{
			PTDSBasicType::tsize size{ 0 }; // 前置指明的数量
			PTDSBasicTypeEnum type{ PTDSBasicTypeEnum::Unknown }; // 前置或后置指明的类型
			::std::vector<AttributeRecord> vset;
		};
		void AnRoot(PTDO& pto, PTDSFile& pts);
		// 传入的 pts 应引用到标志标签起始的方括号'['
		void AnTag(PTDO& pto, PTDSFile& pts, TSP& tsp, TagSet& root_tag_set);
		void AnBlockTag(PTDO& pto, PTDSFile& pts, TSP& tsp, TagSet& root_tag_set);
		void AnType(PTDSFile& pts, PTDSBasicType::tsize& size, PTDSBasicTypeEnum& type);
		void AnRecordTag(PTDSFile& pts, PTDSBasicType::str_utf16le& tag);
		void AnAttrSet(PTDO& pto, PTDSFile& pts, TSP& tsp, AttributeRecordSet& attr_set, bool post_type_aft_block = false);
		bool AttrBegin(const PTDSBasicType::char_utf16le& w);
		// 数字的开头的字符如'0'-'9', '+', '-', '.', 则返回 true
		bool CheckNumberBegin(const PTDSBasicType::char_utf16le& w);
		bool CheckBooleanBegin(const PTDSBasicType::char_utf16le& w);
		// 重复 Tag 非法, 检查 Tag 是否合法, tag_set 中没有传入的 tag 返回 true, 否则返回 false
		bool CheckTagOutSet(const PTDSFile& pts, const TagSet& root_tag_set, const Tag& tag);
		Tag CreateFullTagName(const TSP& tsp);
		void RecordNumberSet(PTDSFile& pts, AttributeRecordSet& dest);
		void RecordNumber(PTDSFile& pts, AttributeRecord& attr);
		void RecordNumberTailB(PTDSFile& pts, PTDSBasicType::str_utf16le& tail);
		void RecordNumberTailH(PTDSFile& pts, PTDSBasicType::str_utf16le& tail);
		void RecordNumberTailD(PTDSFile& pts, AttributeRecord& attr);
		void RecordBooleanSet(PTDSFile& pts, AttributeRecordSet& dest);
		void RecordBoolean(PTDSFile& pts, AttributeRecord& attr);
		void RecordStringSet(PTDSFile& pts, AttributeRecordSet& dest);
		void RecordString(PTDSFile& pts, AttributeRecord& attr);
		// 索引至 '\' 后需要被转义的部分, 如 "abc\rdef" 传入的 pts 应索引至'r'
		bool EscapeCharacter(PTDSFile& pts, PTDSBasicType::str_utf16le& str);
		void RecordPreType(PTDSFile& pts, PTDSBasicTypeEnum& post_type);
		void RecordPostType(PTDSFile& pts, PTDSBasicTypeEnum& post_type);
		void GetFirstSpecifiedType(AttributeRecordSet& attr_set, PTDSBasicTypeEnum& type);
		void GetNumberDefaultType(AttributeRecord& attr, PTDSBasicTypeEnum& type);
		void AnNumberAttrSetToValue(AttributeRecordSet& attr_set, PTDSValueSet& dest);
		void AnNumberAttrToValue(const AttributeRecord& attr, const PTDSBasicTypeEnum& target_type, PTDSValueType& dest);
		// 将 attr.value 部分(不带正负符号, 不带0x和0b前缀) 转化成正数数值
		bool AnNumberDStringToU64(const AttributeRecord& attr, PTDSBasicType::u64& dest);
		bool AnNumberDStringToF32(const AttributeRecord& attr, PTDSBasicType::f32& dest);
		bool AnNumberDStringToF64(const AttributeRecord& attr, PTDSBasicType::f64& dest);
		bool AnNumberBStringToU64(const AttributeRecord& attr, PTDSBasicType::u64& dest);
		bool AnNumberHStringToU64(const AttributeRecord& attr, PTDSBasicType::u64& dest);
		void AnBooleanAttrSetToValue(AttributeRecordSet& attr_set, PTDSValueSet& dest);
		void AnBooleanAttrToValue(const AttributeRecord& attr, const PTDSBasicTypeEnum& target_type, PTDSValueType& dest);
		void AnStringAttrSetToValue(AttributeRecordSet& attr_set, PTDSValueSet& dest);
		void AnStringAttrToValue(const AttributeRecord& attr, const PTDSBasicTypeEnum& target_type, PTDSValueType& dest);
	}
	PTDS::PTDS()
	{
		pto = new PTDO;
	}
	PTDS::~PTDS()
	{
		if (pto != nullptr)
		{
			delete pto;
			pto = nullptr;
		}
	}
	PTDS::PTDS(const PTDS& o)
	{
		pto = new PTDO;
		(*reinterpret_cast<PTDO*>(pto)) = (*reinterpret_cast<PTDO*>(o.pto));
	}
	PTDS::PTDS(PTDS&& o) noexcept
	{
		pto = o.pto;
		o.pto = new PTDO;
		if (pto == nullptr)
		{
			pto = new PTDO;
		}
	}
	PTDS& PTDS::operator=(const PTDS& o)
	{
		(*reinterpret_cast<PTDO*>(pto)) = (*reinterpret_cast<PTDO*>(o.pto));
		return *this;
	}
	void PTDS::LoadPTDS(const PTDSBasicType::char_utf16le* file_name_)
	{
		PTDO& pto = (*(reinterpret_cast<PTDO*>(this->pto)));
		PTDSFile file_pts;
		if (file_pts.Open(file_name_) == false)
		{
			throw PTDSException{ PTDSErrorCode::FailedInOpenFile, PTDSErrorMsg::FailedInOpenFile, static_cast<PTDSBasicType::tsize>(file_pts.LastError()), 0, L'\0' };
			return;
		}
		pto.clear();
		PTDSTools::AnRoot(pto, file_pts);
	}
	void PTDS::LoadPTDSFromOuterBuffer(const PTDSBasicType::char_utf16le* buffer, const PTDSBasicType::tsize& size)
	{
		PTDO& pto = (*(reinterpret_cast<PTDO*>(this->pto)));
		PTDSFile file_pts;
		if (file_pts.RefOuterBuffer(buffer, size) == false)
		{
			throw PTDSException{ PTDSErrorCode::FailedInOpenFile, PTDSErrorMsg::FailedInOpenFile, static_cast<PTDSBasicType::tsize>(file_pts.LastError()), 0, L'\0' };
		}
		pto.clear();
		PTDSTools::AnRoot(pto, file_pts);
	}
	void PTDS::LoadPTDSFromBuffer(const PTDSBasicType::char_utf16le* buffer, const PTDSBasicType::tsize& size)
	{
		PTDO& pto = (*(reinterpret_cast<PTDO*>(this->pto)));
		PTDSFile file_pts;
		if (file_pts.CopyOuterAsBuffer(buffer, size) == false)
		{
			throw PTDSException{ PTDSErrorCode::FailedInOpenFile, PTDSErrorMsg::FailedInOpenFile, static_cast<PTDSBasicType::tsize>(file_pts.LastError()), 0, L'\0' };
		}
		pto.clear();
		PTDSTools::AnRoot(pto, file_pts);
	}
	const PTDO& PTDS::PTDSObject() const
	{
		return (*(reinterpret_cast<PTDO*>(pto)));
	}
	const PTDSValueSet& PTDS::Entity(const WString& tag) const
	{
		PTDO& pto = (*(reinterpret_cast<PTDO*>(this->pto)));
		auto result = pto.find(tag);
		if (result == pto.end())
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::CanNotFindTag, PTDSBasicTypeEnum::Unknown, 0 };
		}
		return result->second;
	}
	const PTDSBasicType::i8& PTDS::ElementI8(const WString& tag, const PTDSBasicType::tsize& index) const
	{
		PTDO& pto = (*(reinterpret_cast<PTDO*>(this->pto)));
		auto result = pto.find(tag);
		if (result == pto.end())
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::CanNotFindTag, PTDSBasicTypeEnum::Unknown, 0 };
		}
		if (result->second.t != PTDSBasicTypeEnum::i8)
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::WrongType, result->second.t, result->second.v.size() };
		}
		if (index >= result->second.v.size())
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::OutOfRange, result->second.t, result->second.v.size() };
		}
		return result->second.v[index].i8;
	}
	const PTDSBasicType::i16& PTDS::ElementI16(const WString& tag, const PTDSBasicType::tsize& index) const
	{
		PTDO& pto = (*(reinterpret_cast<PTDO*>(this->pto)));
		auto result = pto.find(tag);
		if (result == pto.end())
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::CanNotFindTag, PTDSBasicTypeEnum::Unknown, 0 };
		}
		if (result->second.t != PTDSBasicTypeEnum::i16)
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::WrongType, result->second.t, result->second.v.size() };
		}
		if (index >= result->second.v.size())
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::OutOfRange, result->second.t, result->second.v.size() };
		}
		return result->second.v[index].i16;
	}
	const PTDSBasicType::i32& PTDS::ElementI32(const WString& tag, const PTDSBasicType::tsize& index) const
	{
		PTDO& pto = (*(reinterpret_cast<PTDO*>(this->pto)));
		auto result = pto.find(tag);
		if (result == pto.end())
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::CanNotFindTag, PTDSBasicTypeEnum::Unknown, 0 };
		}
		if (result->second.t != PTDSBasicTypeEnum::i32)
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::WrongType, result->second.t, result->second.v.size() };
		}
		if (index >= result->second.v.size())
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::OutOfRange, result->second.t, result->second.v.size() };
		}
		return result->second.v[index].i32;
	}
	const PTDSBasicType::i64& PTDS::ElementI64(const WString& tag, const PTDSBasicType::tsize& index) const
	{
		PTDO& pto = (*(reinterpret_cast<PTDO*>(this->pto)));
		auto result = pto.find(tag);
		if (result == pto.end())
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::CanNotFindTag, PTDSBasicTypeEnum::Unknown, 0 };
		}
		if (result->second.t != PTDSBasicTypeEnum::i64)
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::WrongType, result->second.t, result->second.v.size() };
		}
		if (index >= result->second.v.size())
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::OutOfRange, result->second.t, result->second.v.size() };
		}
		return result->second.v[index].i64;
	}
	const PTDSBasicType::u8& PTDS::ElementU8(const WString& tag, const PTDSBasicType::tsize& index) const
	{
		PTDO& pto = (*(reinterpret_cast<PTDO*>(this->pto)));
		auto result = pto.find(tag);
		if (result == pto.end())
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::CanNotFindTag, PTDSBasicTypeEnum::Unknown, 0 };
		}
		if (result->second.t != PTDSBasicTypeEnum::u8)
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::WrongType, result->second.t, result->second.v.size() };
		}
		if (index >= result->second.v.size())
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::OutOfRange, result->second.t, result->second.v.size() };
		}
		return result->second.v[index].u8;
	}
	const PTDSBasicType::u16& PTDS::ElementU16(const WString& tag, const PTDSBasicType::tsize& index) const
	{
		PTDO& pto = (*(reinterpret_cast<PTDO*>(this->pto)));
		auto result = pto.find(tag);
		if (result == pto.end())
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::CanNotFindTag, PTDSBasicTypeEnum::Unknown, 0 };
		}
		if (result->second.t != PTDSBasicTypeEnum::u16)
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::WrongType, result->second.t, result->second.v.size() };
		}
		if (index >= result->second.v.size())
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::OutOfRange, result->second.t, result->second.v.size() };
		}
		return result->second.v[index].u16;
	}
	const PTDSBasicType::u32& PTDS::ElementU32(const WString& tag, const PTDSBasicType::tsize& index) const
	{
		PTDO& pto = (*(reinterpret_cast<PTDO*>(this->pto)));
		auto result = pto.find(tag);
		if (result == pto.end())
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::CanNotFindTag, PTDSBasicTypeEnum::Unknown, 0 };
		}
		if (result->second.t != PTDSBasicTypeEnum::u32)
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::WrongType, result->second.t, result->second.v.size() };
		}
		if (index >= result->second.v.size())
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::OutOfRange, result->second.t, result->second.v.size() };
		}
		return result->second.v[index].u32;
	}
	const PTDSBasicType::u64& PTDS::ElementU64(const WString& tag, const PTDSBasicType::tsize& index) const
	{
		PTDO& pto = (*(reinterpret_cast<PTDO*>(this->pto)));
		auto result = pto.find(tag);
		if (result == pto.end())
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::CanNotFindTag, PTDSBasicTypeEnum::Unknown, 0 };
		}
		if (result->second.t != PTDSBasicTypeEnum::u64)
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::WrongType, result->second.t, result->second.v.size() };
		}
		if (index >= result->second.v.size())
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::OutOfRange, result->second.t, result->second.v.size() };
		}
		return result->second.v[index].u64;
	}
	const PTDSBasicType::f32& PTDS::ElementF32(const WString& tag, const PTDSBasicType::tsize& index) const
	{
		PTDO& pto = (*(reinterpret_cast<PTDO*>(this->pto)));
		auto result = pto.find(tag);
		if (result == pto.end())
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::CanNotFindTag, PTDSBasicTypeEnum::Unknown, 0 };
		}
		if (result->second.t != PTDSBasicTypeEnum::f32)
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::WrongType, result->second.t, result->second.v.size() };
		}
		if (index >= result->second.v.size())
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::OutOfRange, result->second.t, result->second.v.size() };
		}
		return result->second.v[index].f32;
	}
	const PTDSBasicType::f64& PTDS::ElementF64(const WString& tag, const PTDSBasicType::tsize& index) const
	{
		PTDO& pto = (*(reinterpret_cast<PTDO*>(this->pto)));
		auto result = pto.find(tag);
		if (result == pto.end())
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::CanNotFindTag, PTDSBasicTypeEnum::Unknown, 0 };
		}
		if (result->second.t != PTDSBasicTypeEnum::f64)
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::WrongType, result->second.t, result->second.v.size() };
		}
		if (index >= result->second.v.size())
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::OutOfRange, result->second.t, result->second.v.size() };
		}
		return result->second.v[index].f64;
	}
	const PTDSBasicType::bln& PTDS::ElementBool(const WString& tag, const PTDSBasicType::tsize& index) const
	{
		PTDO& pto = (*(reinterpret_cast<PTDO*>(this->pto)));
		auto result = pto.find(tag);
		if (result == pto.end())
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::CanNotFindTag, PTDSBasicTypeEnum::Unknown, 0 };
		}
		if (result->second.t != PTDSBasicTypeEnum::bln)
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::WrongType, result->second.t, result->second.v.size() };
		}
		if (index >= result->second.v.size())
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::OutOfRange, result->second.t, result->second.v.size() };
		}
		return result->second.v[index].bln;
	}
	const PTDSBasicType::cha& PTDS::ElementChar(const WString& tag, const PTDSBasicType::tsize& index) const
	{
		PTDO& pto = (*(reinterpret_cast<PTDO*>(this->pto)));
		auto result = pto.find(tag);
		if (result == pto.end())
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::CanNotFindTag, PTDSBasicTypeEnum::Unknown, 0 };
		}
		if (result->second.t != PTDSBasicTypeEnum::cha)
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::WrongType, result->second.t, result->second.v.size() };
		}
		if (index >= result->second.v.size())
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::OutOfRange, result->second.t, result->second.v.size() };
		}
		return result->second.v[index].cha;
	}
	const PTDSBasicType::str& PTDS::ElementStr(const WString& tag, const PTDSBasicType::tsize& index) const
	{
		PTDO& pto = (*(reinterpret_cast<PTDO*>(this->pto)));
		auto result = pto.find(tag);
		if (result == pto.end())
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::CanNotFindTag, PTDSBasicTypeEnum::Unknown, 0 };
		}
		if (result->second.t != PTDSBasicTypeEnum::str)
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::WrongType, result->second.t, result->second.v.size() };
		}
		if (index >= result->second.v.size())
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::OutOfRange, result->second.t, result->second.v.size() };
		}
		return result->second.v[index].str;
	}
	const PTDSBasicType::str& PTDS::OriElemStr(const WString& tag, const PTDSBasicType::tsize& index) const
	{
		PTDO& pto = (*(reinterpret_cast<PTDO*>(this->pto)));
		auto result = pto.find(tag);
		if (result == pto.end())
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::CanNotFindTag, PTDSBasicTypeEnum::Unknown, 0 };
		}
		if (index >= result->second.v.size())
		{
			throw PTDSQueryException{ PTDSQueryErrorCode::OutOfRange, result->second.t, result->second.v.size() };
		}
		return result->second.v[index].str;
	}
	namespace PTDSTools
	{
		void AnRoot(PTDO& pto, PTDSFile& pts)
		{
			TagSet local_tag_set;
			TSP tsp;
			while (!pts.EndOfFile())
			{
				NextValid(pts);
				if (pts.EndOfFile())
				{
					break;
				}
				switch (pts.Ref())
				{
				case keyword_tagopen:
					AnTag(pto, pts, tsp, local_tag_set);
					break;
				default:
					throw PTDSException(PTDSErrorCode::IllegalCharacter, PTDSErrorMsg::IllegalCharacter, pts);
					break;
				}
			}
		}
		void AnTag(PTDO& pto, PTDSFile& pts, TSP& tsp, TagSet& root_tag_set)
		{
			if (pts.Ref() == keyword_tagopen)
			{
				pts.Next();
			}
			else
			{
				throw PTDSException(PTDSErrorCode::IllegalCharacter, PTDSErrorMsg::IllegalCharacter, pts);
				return;
			}
			const PTDSBasicType::tsize ser_tag_line{ pts.Line() }; // 标签起始行
			const PTDSBasicType::tsize ser_tag_pos{ pts.Position() }; // 标签起始位置
			PTDSTools::TagSet local_tag_set;
			NextValid(pts); // 引用到标签名第一个字符
			if (pts.EndOfFile())
			{
				throw PTDSException(PTDSErrorCode::CanNotFindTag, PTDSErrorMsg::CanNotFindTag, ser_tag_line, ser_tag_pos, L'\0');
				return;
			}
			PTDSTools::Tag tag;
			AnRecordTag(pts, tag); // 记录 tag, 并检查 tag 是否为空, 空则抛出异常
			if (PTDSTools::CheckTagOutSet(pts, root_tag_set, tag) == false)
			{
				PTDSTools::Tag full{ L": " };
				for (const auto& e : tsp)
				{
					full += e;
					full.push_back(L':');
				}
				full += tag;
				throw PTDSException(PTDSErrorCode::TagNotUnique, (PTDSErrorMsg::TagNotUnique + full).c_str(), pts);
			}
			root_tag_set.insert(tag);
			tsp.push_back(tag);
			NextValid(pts);
			if (pts.EndOfFile())
			{
				throw PTDSException(PTDSErrorCode::TagNotClosed, PTDSErrorMsg::TagNotClosed, ser_tag_line, ser_tag_pos, L'\0');
				return;
			}
			if (pts.Ref() != keyword_tagclose)
			{
				throw PTDSException(PTDSErrorCode::IllegalCharacter, PTDSErrorMsg::IllegalCharacter, pts);
			}
			pts.Next();
			NextValid(pts);
			// 此时 pts 引用到 “[tag]   xyz” 中的x
			// x 可能是 "entity_begin", '<', '{'

			AttributeRecordSet attr_set;

			if (AttrBegin(pts.Ref()))
			{
				AnAttrSet(pto, pts, tsp, attr_set);
			}
			else
			{
				switch (pts.Ref())
				{
				case keyword_typeopen:
				{
					PTDSBasicType::tsize line{ pts.Line() };
					PTDSBasicType::tsize position{ pts.Position() };
					AnType(pts, attr_set.size, attr_set.type);
					NextValid(pts);
					if (pts.EndOfFile() == true)
					{
						throw PTDSException(PTDSErrorCode::CanNotFindEntity, PTDSErrorMsg::CanNotFindEntity, line, position, L'\0');
					}
					if (pts.Ref() == keyword_blockopen)
					{
						pts.Next();
						line = pts.Line();
						position = pts.Position();
						NextValid(pts);
						if (pts.EndOfFile() == true)
						{
							throw PTDSException(PTDSErrorCode::CanNotFindEntity, PTDSErrorMsg::CanNotFindEntity, line, position, L'\0');
						}
						AnAttrSet(pto, pts, tsp, attr_set, true);
					}
					else if (AttrBegin(pts.Ref()))
					{
						AnAttrSet(pto, pts, tsp, attr_set);
					}
					else
					{
						throw PTDSException(PTDSErrorCode::CanNotFindEntity, PTDSErrorMsg::CanNotFindEntity, line, position, L'\0');
					}
				}
				break;
				case keyword_tagopen:
					AnTag(pto, pts, tsp, local_tag_set);
					break;
				case keyword_blockopen:
				{
					const PTDSBasicType::tsize line{ pts.Line() };
					const PTDSBasicType::tsize position{ pts.Position() };
					pts.Next();
					NextValid(pts);
					if (pts.EndOfFile() == true)
					{
						throw PTDSException(PTDSErrorCode::CanNotFindEntity, PTDSErrorMsg::CanNotFindEntity, line, position, L'\0');
					}
					if (AttrBegin(pts.Ref()) == true)
					{
						AnAttrSet(pto, pts, tsp, attr_set, true);
					}
					else if (pts.Ref() == keyword_tagopen)
					{
						AnBlockTag(pto, pts, tsp, local_tag_set);
					}
					else if (pts.Ref() == keyword_blockclose)
					{
						throw PTDSException(PTDSErrorCode::CanNotFindEntity, PTDSErrorMsg::CanNotFindEntity, line, position, L'\0');
					}
					else
					{
						throw PTDSException(PTDSErrorCode::IllegalCharacter, PTDSErrorMsg::IllegalCharacter, pts);
					}
				}
					break;
				default:
					throw PTDSException(PTDSErrorCode::IllegalCharacter, PTDSErrorMsg::IllegalCharacter, pts);
					break;
				}
			}
			tsp.pop_back();
		}
		void AnBlockTag(PTDO& pto, PTDSFile& pts, TSP& tsp, TagSet& root_tag_set)
		{
			PTDSBasicType::tsize line{ pts.Line() };
			PTDSBasicType::tsize position{ pts.Position() };
			while (pts.EndOfFile() == false)
			{
				NextValid(pts);
				if (pts.EndOfFile())
				{
					throw PTDSException(PTDSErrorCode::BlockNotClosed, PTDSErrorMsg::BlockNotClosed, line, position, L'\0');
				}
				if (pts.Ref() == keyword_blockclose)
				{
					pts.Next();
					break;
				}
				else if (pts.Ref() == keyword_tagopen)
				{
					AnTag(pto, pts, tsp, root_tag_set);
				}
				else
				{
					throw PTDSException(PTDSErrorCode::IllegalCharacter, PTDSErrorMsg::IllegalCharacter, pts);
				}
			}
		}
		void AnType(PTDSFile& pts, PTDSBasicType::tsize& size, PTDSBasicTypeEnum& type)
		{
			PTDSBasicType::tsize line{ pts.Line() };
			PTDSBasicType::tsize position{ pts.Position() };
			if (pts.Ref() != keyword_typeopen)
			{
				throw PTDSException(PTDSErrorCode::IllegalCharacter, PTDSErrorMsg::IllegalCharacter, pts);
			}
			pts.Next();
			NextValid(pts);
			if (pts.EndOfFile() == true)
			{
				throw PTDSException(PTDSErrorCode::TypeNotClosed, PTDSErrorMsg::TypeNotClosed, line, position, L'\0');
			}
			if (CheckNumberBegin(pts.Ref()) == true)
			{
				AttributeRecord attr;
				PTDSValueType val;
				RecordNumber(pts, attr);
				AnNumberAttrToValue(attr, PTDSBasicTypeEnum::u64, val);
				if (val.u64 < 1)
				{
					throw PTDSException(PTDSErrorCode::WrongSize, PTDSErrorMsg::WrongSize, pts);
				}
				size = static_cast<PTDSBasicType::tsize>(val.u64);
				NextValid(pts);
				if (pts.EndOfFile() == true)
				{
					throw PTDSException(PTDSErrorCode::TypeNotClosed, PTDSErrorMsg::TypeNotClosed, line, position, L'\0');
				}
				if (pts.Ref() == keyword_separator)
				{
					pts.Next();
					NextValid(pts);
					if (pts.EndOfFile() == true)
					{
						throw PTDSException(PTDSErrorCode::TypeNotClosed, PTDSErrorMsg::TypeNotClosed, line, position, L'\0');
					}
					RecordPreType(pts, type);
					if (type == PTDSBasicTypeEnum::Unknown)
					{
						throw PTDSException(PTDSErrorCode::WrongType, PTDSErrorMsg::WrongType, pts);
					}
					NextValid(pts);
					if (pts.EndOfFile() == true)
					{
						throw PTDSException(PTDSErrorCode::TypeNotClosed, PTDSErrorMsg::TypeNotClosed, line, position, L'\0');
					}
					if (pts.Ref() == keyword_typeclose)
					{
						pts.Next();
						return;
					}
					else
					{
						throw PTDSException(PTDSErrorCode::IllegalCharacter, PTDSErrorMsg::IllegalCharacter, pts);
					}
				}
				else if (pts.Ref() == keyword_typeclose)
				{
					pts.Next();
					return;
				}
				else
				{
					throw PTDSException(PTDSErrorCode::TypeNotClosed, PTDSErrorMsg::TypeNotClosed, line, position, L'\0');
				}
			}
			else
			{
				RecordPreType(pts, type);
				if (type == PTDSBasicTypeEnum::Unknown)
				{
					throw PTDSException(PTDSErrorCode::WrongType, PTDSErrorMsg::WrongType, pts);
				}
				NextValid(pts);
				if (pts.EndOfFile() == true)
				{
					throw PTDSException(PTDSErrorCode::TypeNotClosed, PTDSErrorMsg::TypeNotClosed, line, position, L'\0');
				}
				if (pts.Ref() == keyword_separator)
				{
					pts.Next();
					NextValid(pts);
					if (pts.EndOfFile() == true)
					{
						throw PTDSException(PTDSErrorCode::TypeNotClosed, PTDSErrorMsg::TypeNotClosed, line, position, L'\0');
					}
					AttributeRecord attr;
					PTDSValueType val;
					RecordNumber(pts, attr);
					AnNumberAttrToValue(attr, PTDSBasicTypeEnum::u64, val);
					if (val.u64 < 1)
					{
						throw PTDSException(PTDSErrorCode::WrongSize, PTDSErrorMsg::WrongSize, pts);
					}
					size = static_cast<PTDSBasicType::tsize>(val.u64);
					NextValid(pts);
					if (pts.EndOfFile() == true)
					{
						throw PTDSException(PTDSErrorCode::TypeNotClosed, PTDSErrorMsg::TypeNotClosed, line, position, L'\0');
					}
					if (pts.Ref() == keyword_typeclose)
					{
						pts.Next();
						return;
					}
					else
					{
						throw PTDSException(PTDSErrorCode::IllegalCharacter, PTDSErrorMsg::IllegalCharacter, pts);
					}
				}
				else if (pts.Ref() == keyword_typeclose)
				{
					pts.Next();
					return;
				}
				else
				{
					throw PTDSException(PTDSErrorCode::IllegalCharacter, PTDSErrorMsg::IllegalCharacter, pts);
				}
			}
		}
		void AnRecordTag(PTDSFile& pts, PTDSBasicType::str_utf16le& tag)
		{
			const PTDSBasicType::tsize line{ pts.Line() };
			const PTDSBasicType::tsize position{ pts.Position() };
			tag.clear();
			while (!pts.EndOfFile())
			{
				switch (pts.Ref())
				{
				case L':':
				case keyword_tagopen:
				case keyword_tagclose:
				case keyword_typeopen:
				case keyword_typeclose:
				case keyword_blockopen:
				case keyword_blockclose:
				case keyword_ex_bracketopen:
				case keyword_ex_bracketclose:
				case keyword_ex_div:
				case L'\\':
				case keyword_quote:
				case keyword_separator:
					goto _pts_OUT_OF_LOOP;
					break;
				default:
					break;
				}
				if (pts.CheckValid() == false)
				{
					break;
				}
				tag.push_back(pts.Ref());
				pts.Next();
			}
		_pts_OUT_OF_LOOP:
			if (tag.length() <= 0)
			{
				throw PTDSException(PTDSErrorCode::CanNotFindTag, PTDSErrorMsg::CanNotFindTag, line, position, L'\0');
			}
			return;
		}
		void AnAttrSet(PTDO& pto, PTDSFile& pts, TSP& tsp, AttributeRecordSet& attr_set, bool post_type_aft_block)
		{
			auto check_type_aft_block = []
			(PTDSFile& pts, AttributeRecordSet& attr_set, const PTDSBasicType::tsize& line, const PTDSBasicType::tsize& position) -> void
			{
				NextValid(pts);
				if (pts.EndOfFile() == true || pts.Ref() != keyword_blockclose)
				{
					throw PTDSException(PTDSErrorCode::BlockNotClosed, PTDSErrorMsg::BlockNotClosed, line, position, L'\0');
				}
				pts.Next();
				PTDSBasicTypeEnum post_type{ PTDSBasicTypeEnum::Unknown };
				NextValid(pts);
				if (pts.EndOfFile() == false)
				{
					RecordPostType(pts, post_type);
					NextValid(pts);
					if (post_type != PTDSBasicTypeEnum::Unknown)
					{
						if (attr_set.type == PTDSBasicTypeEnum::Unknown)
						{
							attr_set.type = post_type;
						}
						else
						{
							if (attr_set.type != post_type)
							{
								throw PTDSException(PTDSErrorCode::WrongType, PTDSErrorMsg::WrongType, pts);
							}
						}
					}
				}
			};
			PTDSBasicType::tsize line{ pts.Line() };
			PTDSBasicType::tsize position{ pts.Position() };
			if (PTDSTools::CheckBooleanBegin(pts.Ref()) || attr_set.type == PTDSBasicTypeEnum::bln)
			{
				const PTDSBasicType::tsize line{ pts.Line() };
				const PTDSBasicType::tsize position{ pts.Position() };
				PTDSValueSet value_set;
				PTDSTools::Tag full{ CreateFullTagName(tsp) };
				RecordBooleanSet(pts, attr_set);
				if (attr_set.vset.size() < 1)
				{
					throw PTDSException(PTDSErrorCode::CanNotFindEntity, PTDSErrorMsg::CanNotFindEntity, line, position, L'\0');
				}
				if (post_type_aft_block)
				{
					check_type_aft_block(pts, attr_set, line, position);
				}
				if (attr_set.type == PTDSBasicTypeEnum::Unknown)
				{
					attr_set.type = PTDSBasicTypeEnum::bln;
				}
				AnBooleanAttrSetToValue(attr_set, value_set);
				if (attr_set.size > 0)
				{
					if (value_set.v.size() != attr_set.size)
					{
						throw PTDSException(PTDSErrorCode::WrongSize, PTDSErrorMsg::WrongSize, pts);
					}
				}
				pto[full] = ::std::move(value_set);
			}
			else if (pts.Ref() == keyword_quote || (attr_set.type == PTDSBasicTypeEnum::str || attr_set.type == PTDSBasicTypeEnum::cha))
			{
				PTDSValueSet value_set;
				PTDSTools::Tag full{ CreateFullTagName(tsp) };
				RecordStringSet(pts, attr_set);
				if (post_type_aft_block)
				{
					check_type_aft_block(pts, attr_set, line, position);
				}
				if (attr_set.type == PTDSBasicTypeEnum::Unknown)
				{
					GetFirstSpecifiedType(attr_set, attr_set.type);
					if (attr_set.type == PTDSBasicTypeEnum::Unknown)
					{
						attr_set.type = PTDSBasicTypeEnum::str;
					}
				}
				AnStringAttrSetToValue(attr_set, value_set);
				if (attr_set.size > 0)
				{
					if (value_set.v.size() != attr_set.size)
					{
						throw PTDSException(PTDSErrorCode::WrongSize, PTDSErrorMsg::WrongSize, pts);
					}
				}
				pto[full] = ::std::move(value_set);
			}
			else if (PTDSTools::CheckNumberBegin(pts.Ref()))
			{
				const PTDSBasicType::tsize line{ pts.Line() };
				const PTDSBasicType::tsize position{ pts.Position() };
				PTDSValueSet value_set;
				PTDSTools::Tag full{ CreateFullTagName(tsp) };
				RecordNumberSet(pts, attr_set);
				if (attr_set.vset.size() < 1)
				{
					throw PTDSException(PTDSErrorCode::CanNotFindEntity, PTDSErrorMsg::CanNotFindEntity, line, position, L'\0');
				}
				if (post_type_aft_block)
				{
					check_type_aft_block(pts, attr_set, line, position);
				}
				if (attr_set.type == PTDSBasicTypeEnum::Unknown)
				{
					GetFirstSpecifiedType(attr_set, attr_set.type);
					if (attr_set.type == PTDSBasicTypeEnum::Unknown)
					{
						GetNumberDefaultType(attr_set.vset[0], attr_set.type);
					}
				}
				AnNumberAttrSetToValue(attr_set, value_set);
				if (attr_set.size > 0)
				{
					if (value_set.v.size() != attr_set.size)
					{
						throw PTDSException(PTDSErrorCode::WrongSize, PTDSErrorMsg::WrongSize, pts);
					}
				}
				pto[full] = ::std::move(value_set);
			}
			else
			{
				throw PTDSException(PTDSErrorCode::IllegalCharacter, PTDSErrorMsg::IllegalCharacter, pts);
			}
		}
		bool AttrBegin(const PTDSBasicType::char_utf16le& w)
		{
			if (CheckNumberBegin(w) || CheckBooleanBegin(w) || w == keyword_quote)
			{
				return true;
			}
			return false;
		}
		bool CheckNumberBegin(const PTDSBasicType::char_utf16le& w)
		{
			;
			if ((w >= L'0' && w <= L'9') ||
				(w == L'+') ||
				(w == L'-') ||
				(w == L'.'))
			{
				return true;
			}
			return false;
		}
		bool CheckBooleanBegin(const PTDSBasicType::char_utf16le& w)
		{
			if (w == L'T' || w == L'F' || w == L't' || w == L'f')
			{
				return true;
			}
			return false;
		}
		bool CheckTagOutSet(const PTDSFile& pts, const TagSet& root_tag_set, const Tag& tag)
		{
			if (root_tag_set.find(tag) == root_tag_set.end())
			{
				return true;
			}
			return false;
		}
		Tag CreateFullTagName(const TSP& tsp)
		{
			Tag full;
			for (const auto& e : tsp)
			{
				full += e;
				full.push_back(keyword_tagconnect);
			}
			full.pop_back();
			return full;
		}
		void RecordNumberSet(PTDSFile& pts, AttributeRecordSet& dest)
		{
			while (pts.EndOfFile() == false)
			{
				AttributeRecord attr;
				RecordNumber(pts, attr);
				NextValid(pts);
				if (pts.EndOfFile() == false)
				{
					RecordPostType(pts, attr.type);
					NextValid(pts);
				}
				dest.vset.push_back(::std::move(attr));
				if (pts.Ref() == L',')
				{
					pts.Next();
					NextValid(pts);
					if (CheckNumberBegin(pts.Ref()) == false)
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
		}
		void RecordNumber(PTDSFile& pts, AttributeRecord& attr)
		{
			attr.line = pts.Line();
			attr.position = pts.Position();
			attr.positive = true;
			switch (pts.Ref())
			{
			case L'+':
				attr.sign = L"+";
				attr.positive = true;
				pts.Next();
				NextValid(pts);
				break;
			case L'-':
				attr.sign = L"-";
				attr.positive = false;
				pts.Next();
				NextValid(pts);
				break;
			default:
				break;
			}
			if (pts.EndOfFile() != false)
			{
				throw PTDSException(PTDSErrorCode::CanNotFindEntity, PTDSErrorMsg::CanNotFindEntity, attr.line, attr.position, L'\0');
			}
			if (!(pts.Ref() >= L'0' && pts.Ref() <= L'9' || pts.Ref() == L'.'))
			{
				throw PTDSException(PTDSErrorCode::IllegalCharacter, PTDSErrorMsg::IllegalCharacter, pts);
			}
			if (pts.Ref() == L'0')
			{
				PTDSFileErrorCode error{ PTDSFileErrorCode::Unknown };
				const auto& ref1{ pts.Ref(1, error) };
				if (error == PTDSFileErrorCode::Unknown)
				{
					if (ref1 == L'B' || ref1 == L'b')
					{
						attr.bin_open = true;
						attr.format.push_back(pts.Ref());
						attr.format.push_back(ref1);
						pts.Next();
						pts.Next();
					}
					else if (ref1 == L'X' || ref1 == L'x')
					{
						attr.hex_open = true;
						attr.format.push_back(pts.Ref());
						attr.format.push_back(ref1);
						pts.Next();
						pts.Next();
					}
					else if (ref1 >= L'0' && ref1 <= L'9')
					{
						throw PTDSException(PTDSErrorCode::NotNumber, PTDSErrorMsg::NotNumber, pts);
					}
				}
			}
			if (attr.bin_open == true)
			{
				RecordNumberTailB(pts, attr.value);
			}
			else if (attr.hex_open == true)
			{
				RecordNumberTailH(pts, attr.value);
			}
			else
			{
				RecordNumberTailD(pts, attr);
			}
			if (attr.value.length() < 1)
			{
				throw PTDSException(PTDSErrorCode::NotNumber, PTDSErrorMsg::NotNumber, attr.line, attr.position, L'\0');
			}
		}
		void RecordNumberTailB(PTDSFile& pts, PTDSBasicType::str_utf16le& tail)
		{
			while (pts.EndOfFile() == false)
			{
				if (pts.Ref() >= L'0' && pts.Ref() <= L'1')
				{
					tail.push_back(pts.Ref());
					pts.Next();
				}
				else
				{
					break;
				}
			}
		}
		void RecordNumberTailH(PTDSFile& pts, PTDSBasicType::str_utf16le& tail)
		{
			while (pts.EndOfFile() == false)
			{
				if ((pts.Ref() >= L'0' && pts.Ref() <= L'9') ||
					(pts.Ref() >= L'A' && pts.Ref() <= L'F') ||
					(pts.Ref() >= L'a' && pts.Ref() <= L'f'))
				{
					tail.push_back(pts.Ref());
					pts.Next();
				}
				else
				{
					break;
				}
			}
		}
		void RecordNumberTailD(PTDSFile& pts, AttributeRecord& attr)
		{
			bool no_num_before_dot{ false };
			auto& tail{ attr.value };
			if (pts.Ref() == L'.')
			{
				no_num_before_dot = true;
			}
			while (pts.EndOfFile() == false)
			{
				if (pts.Ref() >= L'0' && pts.Ref() <= L'9')
				{
					tail.push_back(pts.Ref());
					pts.Next();
				}
				else if (pts.Ref() == L'.')
				{
					tail.push_back(pts.Ref());
					attr.float_str = true;
					pts.Next();
					if (no_num_before_dot)
					{
						PTDSBasicType::str_utf16le tail_tail;
						while (pts.EndOfFile() == false)
						{
							if (pts.Ref() >= L'0' && pts.Ref() <= L'9')
							{
								tail_tail.push_back(pts.Ref());
								pts.Next();
							}
							else
							{
								break;
							}
						}
						if (tail_tail.length() < 1)
						{
							throw PTDSException(PTDSErrorCode::NotNumber, PTDSErrorMsg::NotNumber, pts);
						}
						tail += tail_tail;
					}
					else
					{
						while (pts.EndOfFile() == false)
						{
							if (pts.Ref() >= L'0' && pts.Ref() <= L'9')
							{
								tail.push_back(pts.Ref());
								pts.Next();
							}
							else
							{
								break;
							}
						}
					}
					break;
				}
				else
				{
					break;
				}
			}
		}
		void RecordBooleanSet(PTDSFile& pts, AttributeRecordSet& dest)
		{
			while (pts.EndOfFile() == false)
			{
				AttributeRecord attr;
				if (CheckNumberBegin(pts.Ref()))
				{
					RecordNumber(pts, attr);
				}
				else if (CheckBooleanBegin(pts.Ref()))
				{
					RecordBoolean(pts, attr);
				}
				else
				{
					throw PTDSException(PTDSErrorCode::IllegalCharacter, PTDSErrorMsg::IllegalCharacter, pts);
				}
				NextValid(pts);
				if (pts.EndOfFile() == false)
				{
					RecordPostType(pts, attr.type);
					NextValid(pts);
				}
				dest.vset.push_back(::std::move(attr));
				if (pts.Ref() == L',')
				{
					pts.Next();
					NextValid(pts);
					if (CheckNumberBegin(pts.Ref()) == false && CheckBooleanBegin(pts.Ref()) == false)
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
		}
		void RecordBoolean(PTDSFile& pts, AttributeRecord& attr)
		{
			attr.line = pts.Line();
			attr.position = pts.Position();
			auto& tail{ attr.value };
			switch (pts.Ref())
			{
			case L'T':
			case L't':
				tail.push_back(pts.Ref());
				pts.Next();
				if (pts.EndOfFile() == false && (pts.Ref() == L'R' || pts.Ref() == L'r'))
				{
					PTDSFileErrorCode error1{ PTDSFileErrorCode::Unknown };
					PTDSFileErrorCode error2{ PTDSFileErrorCode::Unknown };
					const auto& ref1{ pts.Ref(1, error1) };
					const auto& ref2{ pts.Ref(2, error2) };
					if (error1 == PTDSFileErrorCode::Unknown && error2 == PTDSFileErrorCode::Unknown)
					{
						if ((ref1 == L'U' || ref1 == L'u') &&
							(ref2 == L'E' || ref2 == L'e'))
						{
							tail.push_back(pts.Ref());
							pts.Next();
							tail.push_back(pts.Ref());
							pts.Next();
							tail.push_back(pts.Ref());
							pts.Next();
						}
					}
				}
				break;
			case L'F':
			case L'f':
				tail.push_back(pts.Ref());
				pts.Next();
				if (pts.EndOfFile() == false && (pts.Ref() == L'A' || pts.Ref() == L'a'))
				{
					PTDSFileErrorCode error1{ PTDSFileErrorCode::Unknown };
					PTDSFileErrorCode error2{ PTDSFileErrorCode::Unknown };
					PTDSFileErrorCode error3{ PTDSFileErrorCode::Unknown };
					const auto& ref1{ pts.Ref(1, error1) };
					const auto& ref2{ pts.Ref(2, error2) };
					const auto& ref3{ pts.Ref(3, error3) };
					if (error1 == PTDSFileErrorCode::Unknown &&
						error2 == PTDSFileErrorCode::Unknown &&
						error3 == PTDSFileErrorCode::Unknown)
					{
						if ((ref1 == L'L' || ref1 == L'l') &&
							(ref2 == L'S' || ref2 == L's') &&
							(ref3 == L'E' || ref3 == L'e'))
						{
							tail.push_back(pts.Ref());
							pts.Next();
							tail.push_back(pts.Ref());
							pts.Next();
							tail.push_back(pts.Ref());
							pts.Next();
							tail.push_back(pts.Ref());
							pts.Next();
						}
					}
				}
				break;
			default:
				throw PTDSException(PTDSErrorCode::IllegalCharacter, PTDSErrorMsg::IllegalCharacter, pts);
				break;
			}
		}
		void RecordStringSet(PTDSFile& pts, AttributeRecordSet& dest)
		{
			while (pts.EndOfFile() == false)
			{
				AttributeRecord attr;
				if (pts.Ref() != keyword_quote)
				{
					throw PTDSException(PTDSErrorCode::IllegalCharacter, PTDSErrorMsg::IllegalCharacter, pts);
				}
				pts.Next();
				RecordString(pts, attr);
				if (pts.Ref() != keyword_quote)
				{
					throw PTDSException(PTDSErrorCode::IllegalCharacter, PTDSErrorMsg::IllegalCharacter, pts);
				}
				pts.Next();
				NextValid(pts);
				if (pts.EndOfFile() == false)
				{
					RecordPostType(pts, attr.type);
					NextValid(pts);
				}
				dest.vset.push_back(::std::move(attr));
				if (pts.EndOfFile() == false && pts.Ref() == L',')
				{
					pts.Next();
					NextValid(pts);
					if (pts.EndOfFile() == true || pts.Ref() != keyword_quote)
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
		}
		void RecordString(PTDSFile& pts, AttributeRecord& attr)
		{
			attr.line = pts.Line();
			attr.position = pts.Position();
			auto& str{ attr.value };
			while (pts.EndOfFile() == false)
			{
				switch (pts.Ref())
				{
				case keyword_quote:
					goto _pts_OUT_OF_LOOP;
					break;
				case L'\\':
					pts.Next();
					EscapeCharacter(pts, str);
					break;
				case ig_cr:
					str.push_back(pts.Ref());
					pts.Next();
					if (pts.EndOfFile() == false && pts.Ref() == ig_lf)
					{
						str.push_back(pts.Ref());
						pts.Next();
					}
					pts.AddLine();
					break;
				case ig_lf:
					str.push_back(pts.Ref());
					pts.Next();
					pts.AddLine();
					break;
				default:
					str.push_back(pts.Ref());
					pts.Next();
					break;
				}
			}
			throw PTDSException(PTDSErrorCode::QuoteNotClosed, PTDSErrorMsg::QuoteNotClosed, attr.line, attr.position, L'\0');
		_pts_OUT_OF_LOOP:
			return;
		}
		bool EscapeCharacter(PTDSFile& pts, PTDSBasicType::str_utf16le& str)
		{
			auto turn_to_int = []
			(const PTDSBasicType::char_utf16le& w, ::std::vector<int>& hex_num) -> bool
			{
				if (w >= L'0' && w <= L'9')
				{
					hex_num.push_back(w - L'0');
				}
				else if (w >= L'A' && w <= L'F')
				{
					hex_num.push_back(w - L'A' + 10);
				}
				else if (w >= L'a' && w <= L'f')
				{
					hex_num.push_back(w - L'a' + 10);
				}
				else
				{
					return false;
				}
				return true;
			};
			::std::vector<int> hex_num;
			PTDSBasicType::char_utf16le v{ 0 };
			bool success{ true };
			if (pts.EndOfFile() == true)
			{
				return false;
			}
			switch (pts.Ref())
			{
			case ig_cr:
				pts.Next();
				if (pts.EndOfFile() == false && pts.Ref() == ig_lf)
				{
					pts.Next();
				}
				pts.AddLine();
				return true;
				break;
			case ig_lf:
				pts.Next();
				pts.AddLine();
				return true;
				break;
			case L'0':
				v = L'\0';
				break;
			case L'a':
				v = L'\a';
				break;
			case L'b':
				v = L'\b';
				break;
			case L'f':
				v = L'\f';
				break;
			case L't':
				v = L'\t';
				break;
			case L'v':
				v = L'\v';
				break;
			case L'n':
				v = L'\n';
				break;
			case L'r':
				v = L'\r';
				break;
			case L'\'':
				v = L'\'';
				break;
			case L'\"':
				v = L'\"';
				break;
			case L'\?':
				v = L'\?';
				break;
			case L'\\':
				v = L'\\';
				break;
			case L'x': //  \x1234
			{
				PTDSFileErrorCode error1{ PTDSFileErrorCode::Unknown };
				PTDSFileErrorCode error2{ PTDSFileErrorCode::Unknown };
				PTDSFileErrorCode error3{ PTDSFileErrorCode::Unknown };
				PTDSFileErrorCode error4{ PTDSFileErrorCode::Unknown };
				const PTDSBasicType::char_utf16le& ref1{ pts.Ref(1, error1) };
				const PTDSBasicType::char_utf16le& ref2{ pts.Ref(2, error2) };
				const PTDSBasicType::char_utf16le& ref3{ pts.Ref(3, error3) };
				const PTDSBasicType::char_utf16le& ref4{ pts.Ref(4, error4) };
				if (error1 == PTDSFileErrorCode::Unknown)
				{
					if (turn_to_int(ref1, hex_num) == false)
					{
						success = false;
						break;
					}
				}
				else
				{
					success = false;
					break;
				}
				if (error2 == PTDSFileErrorCode::Unknown && turn_to_int(ref2, hex_num) == true)
				{
					if (error3 == PTDSFileErrorCode::Unknown && turn_to_int(ref3, hex_num) == true)
					{
						if (error4 == PTDSFileErrorCode::Unknown)
						{
							turn_to_int(ref4, hex_num);
						}
					}
				}
				for (size_t i = 0; i < hex_num.size(); ++i)
				{
					pts.Next();
				}
				if (success)
				{
					int offset{ 0 };
					for (auto it = hex_num.rbegin(); it != hex_num.rend(); ++it)
					{
						v |= ((*it) << offset);
						offset += 4;
					}
				}
			}
			// 读取一个最多4位十六进制数字表示字符的值
			// 如 \xFffF -> 0xffff, \x100PPPPP -> 0x100
			break;
			default:
				success = false;
				break;
			}
			if (success)
			{
				str.push_back(v);
				pts.Next();
			}
			else
			{
				return false;
			}
			return true;
		}
		void RecordPreType(PTDSFile& pts, PTDSBasicTypeEnum& pre_type)
		{
			pre_type = PTDSBasicTypeEnum::Unknown;
			switch (pts.Ref())
			{
			case L'U':
			case L'u':
				pts.Next();
				if (pts.EndOfFile() == false)
				{
					if (pts.Ref() == L'8')
					{
						pre_type = PTDSBasicTypeEnum::u8;
						pts.Next();
						return;
					}
					PTDSFileErrorCode error1{ PTDSFileErrorCode::Unknown };
					const auto& ref1{ pts.Ref(1, error1) };
					switch (pts.Ref())
					{
					case L'1':
						if (ref1 == L'6' && error1 == PTDSFileErrorCode::Unknown)
						{
							pre_type = PTDSBasicTypeEnum::u16;
							pts.Next();
							pts.Next();
							return;
						}
						break;
					case L'3':
						if (ref1 == L'2' && error1 == PTDSFileErrorCode::Unknown)
						{
							pre_type = PTDSBasicTypeEnum::u32;
							pts.Next();
							pts.Next();
							return;
						}
						break;
					case L'6':
						if (ref1 == L'4' && error1 == PTDSFileErrorCode::Unknown)
						{
							pre_type = PTDSBasicTypeEnum::u64;
							pts.Next();
							pts.Next();
							return;
						}
						break;
					default:
						break;
					}
				}
				pre_type = PTDSBasicTypeEnum::u64;
				return;
				break;
			case L'I':
			case L'i':
				pts.Next();
				if (pts.EndOfFile() == false)
				{
					if (pts.Ref() == L'8')
					{
						pre_type = PTDSBasicTypeEnum::i8;
						pts.Next();
						return;
					}
					PTDSFileErrorCode error1{ PTDSFileErrorCode::Unknown };
					const auto& ref1{ pts.Ref(1, error1) };
					switch (pts.Ref())
					{
					case L'1':
						if (ref1 == L'6' && error1 == PTDSFileErrorCode::Unknown)
						{
							pre_type = PTDSBasicTypeEnum::i16;
							pts.Next();
							pts.Next();
							return;
						}
						break;
					case L'3':
						if (ref1 == L'2' && error1 == PTDSFileErrorCode::Unknown)
						{
							pre_type = PTDSBasicTypeEnum::i32;
							pts.Next();
							pts.Next();
							return;
						}
						break;
					case L'6':
						if (ref1 == L'4' && error1 == PTDSFileErrorCode::Unknown)
						{
							pre_type = PTDSBasicTypeEnum::i64;
							pts.Next();
							pts.Next();
							return;
						}
						break;
					default:
						break;
					}
				}
				pre_type = PTDSBasicTypeEnum::i64;
				return;
				break;
			case L'F':
			case L'f':
			{
				PTDSFileErrorCode error1{ PTDSFileErrorCode::Unknown };
				PTDSFileErrorCode error2{ PTDSFileErrorCode::Unknown };
				const auto& ref1{ pts.Ref(1, error1) };
				const auto& ref2{ pts.Ref(2, error2) };
				if (error1 != PTDSFileErrorCode::Unknown || error2 != PTDSFileErrorCode::Unknown)
				{
					throw PTDSException(PTDSErrorCode::IllegalCharacter, PTDSErrorMsg::IllegalCharacter, pts);
				}
				if (ref1 == L'3' && ref2 == L'2')
				{
					pre_type = PTDSBasicTypeEnum::f32;
				}
				else if (ref1 == L'6' && ref2 == L'4')
				{
					pre_type = PTDSBasicTypeEnum::f32;
				}
				else
				{
					throw PTDSException(PTDSErrorCode::IllegalCharacter, PTDSErrorMsg::IllegalCharacter, pts);
				}
				pts.Next();
				pts.Next();
				pts.Next();
				return;
			}
			throw PTDSException(PTDSErrorCode::IllegalCharacter, PTDSErrorMsg::IllegalCharacter, pts);
			break;
			case L'C':
			case L'c':
			{
				PTDSFileErrorCode error1{ PTDSFileErrorCode::Unknown };
				PTDSFileErrorCode error2{ PTDSFileErrorCode::Unknown };
				PTDSFileErrorCode error3{ PTDSFileErrorCode::Unknown };
				const auto& ref1{ pts.Ref(1, error1) };
				const auto& ref2{ pts.Ref(2, error2) };
				const auto& ref3{ pts.Ref(3, error3) };
				if (error1 == PTDSFileErrorCode::Unknown && (ref1 == L'H' || ref1 == L'h') &&
					error2 == PTDSFileErrorCode::Unknown && (ref2 == L'A' || ref2 == L'a') &&
					error3 == PTDSFileErrorCode::Unknown && (ref3 == L'R' || ref3 == L'r'))
				{
					pts.Next();
					pts.Next();
					pts.Next();
					pts.Next();
					pre_type = PTDSBasicTypeEnum::cha;
					return;
				}
			}
			throw PTDSException(PTDSErrorCode::IllegalCharacter, PTDSErrorMsg::IllegalCharacter, pts);
			break;
			case L'S':
			case L's':
			{
				PTDSFileErrorCode error1{ PTDSFileErrorCode::Unknown };
				PTDSFileErrorCode error2{ PTDSFileErrorCode::Unknown };
				const auto& ref1{ pts.Ref(1, error1) };
				const auto& ref2{ pts.Ref(2, error2) };
				if (error1 != PTDSFileErrorCode::Unknown || error2 != PTDSFileErrorCode::Unknown)
				{
					throw PTDSException(PTDSErrorCode::IllegalCharacter, PTDSErrorMsg::IllegalCharacter, pts);
				}
				if ((ref1 == L'T' || ref1 == L't') && (ref2 == L'R' || ref2 == L'r'))
				{
					pre_type = PTDSBasicTypeEnum::str;
					pts.Next();
					pts.Next();
					pts.Next();
					return;
				}
			}
			throw PTDSException(PTDSErrorCode::IllegalCharacter, PTDSErrorMsg::IllegalCharacter, pts);
			break;
			case L'B':
			case L'b':
			{
				PTDSFileErrorCode error1{ PTDSFileErrorCode::Unknown };
				PTDSFileErrorCode error2{ PTDSFileErrorCode::Unknown };
				PTDSFileErrorCode error3{ PTDSFileErrorCode::Unknown };
				const auto& ref1{ pts.Ref(1, error1) };
				const auto& ref2{ pts.Ref(2, error2) };
				const auto& ref3{ pts.Ref(3, error3) };
				if (error1 == PTDSFileErrorCode::Unknown && (ref1 == L'O' || ref1 == L'o') &&
					error2 == PTDSFileErrorCode::Unknown && (ref2 == L'O' || ref2 == L'o') &&
					error3 == PTDSFileErrorCode::Unknown && (ref3 == L'L' || ref3 == L'l'))
				{
					pts.Next();
					pts.Next();
					pts.Next();
					pts.Next();
					pre_type = PTDSBasicTypeEnum::bln;
					return;
				}
			}
			throw PTDSException(PTDSErrorCode::IllegalCharacter, PTDSErrorMsg::IllegalCharacter, pts);
			break;
			default:
				return;
				break;
			}
		}
		void RecordPostType(PTDSFile& pts, PTDSBasicTypeEnum& post_type)
		{
			post_type = PTDSBasicTypeEnum::Unknown;
			switch (pts.Ref())
			{
			case L'U':
			case L'u':
				pts.Next();
				if (pts.EndOfFile() == false)
				{
					if (pts.Ref() == L'8')
					{
						post_type = PTDSBasicTypeEnum::u8;
						pts.Next();
						return;
					}
					PTDSFileErrorCode error1{ PTDSFileErrorCode::Unknown };
					const auto& ref1{ pts.Ref(1, error1) };
					switch (pts.Ref())
					{
					case L'1':
						if (ref1 == L'6' && error1 == PTDSFileErrorCode::Unknown)
						{
							post_type = PTDSBasicTypeEnum::u16;
							pts.Next();
							pts.Next();
							return;
						}
						break;
					case L'3':
						if (ref1 == L'2' && error1 == PTDSFileErrorCode::Unknown)
						{
							post_type = PTDSBasicTypeEnum::u32;
							pts.Next();
							pts.Next();
							return;
						}
						break;
					case L'6':
						if (ref1 == L'4' && error1 == PTDSFileErrorCode::Unknown)
						{
							post_type = PTDSBasicTypeEnum::u64;
							pts.Next();
							pts.Next();
							return;
						}
						break;
					default:
						break;
					}
				}
				post_type = PTDSBasicTypeEnum::u64;
				return;
				break;
			case L'I':
			case L'i':
				pts.Next();
				if (pts.EndOfFile() == false)
				{
					if (pts.Ref() == L'8')
					{
						post_type = PTDSBasicTypeEnum::i8;
						pts.Next();
						return;
					}
					PTDSFileErrorCode error1{ PTDSFileErrorCode::Unknown };
					const auto& ref1{ pts.Ref(1, error1) };
					switch (pts.Ref())
					{
					case L'1':
						if (ref1 == L'6' && error1 == PTDSFileErrorCode::Unknown)
						{
							post_type = PTDSBasicTypeEnum::i16;
							pts.Next();
							pts.Next();
							return;
						}
						break;
					case L'3':
						if (ref1 == L'2' && error1 == PTDSFileErrorCode::Unknown)
						{
							post_type = PTDSBasicTypeEnum::i32;
							pts.Next();
							pts.Next();
							return;
						}
						break;
					case L'6':
						if (ref1 == L'4' && error1 == PTDSFileErrorCode::Unknown)
						{
							post_type = PTDSBasicTypeEnum::i64;
							pts.Next();
							pts.Next();
							return;
						}
						break;
					default:
						break;
					}
				}
				post_type = PTDSBasicTypeEnum::i64;
				return;
				break;
			case L'F':
			case L'f':
				pts.Next();
				if (pts.EndOfFile() == false)
				{
					PTDSFileErrorCode error1{ PTDSFileErrorCode::Unknown };
					const auto& ref1{ pts.Ref(1, error1) };
					if (error1 != PTDSFileErrorCode::Unknown)
					{
						post_type = PTDSBasicTypeEnum::f32;
						return;
					}
					switch (pts.Ref())
					{
					case L'3':
						if (ref1 == L'2')
						{
							post_type = PTDSBasicTypeEnum::f32;
							pts.Next();
							pts.Next();
							return;
						}
						break;
					case L'6':
						if (ref1 == L'4')
						{
							post_type = PTDSBasicTypeEnum::f64;
							pts.Next();
							pts.Next();
							return;
						}
						break;
					default:
						break;
					}
				}
				post_type = PTDSBasicTypeEnum::f32;
				return;
				break;
			case L'C':
			case L'c':
				pts.Next();
				post_type = PTDSBasicTypeEnum::cha;
				return;
				break;
			case L'S':
			case L's':
				pts.Next();
				if (pts.EndOfFile() == false)
				{
					PTDSFileErrorCode error1{ PTDSFileErrorCode::Unknown };
					const auto& ref1{ pts.Ref(1, error1) };
					if (error1 == PTDSFileErrorCode::Unknown)
					{
						if ((pts.Ref() == L'T' || pts.Ref() == L't') &&
							(ref1 == L'R' || ref1 == L'r'))
						{
							pts.Next();
							pts.Next();
							post_type = PTDSBasicTypeEnum::str;
							return;
						}
					}
				}
				post_type = PTDSBasicTypeEnum::str;
				return;
				break;
			default:
				return;
				break;
			}
		}
		void GetFirstSpecifiedType(AttributeRecordSet& attr_set, PTDSBasicTypeEnum& type)
		{
			type = PTDSBasicTypeEnum::Unknown;
			for (const auto& e : attr_set.vset)
			{
				if (e.type != PTDSBasicTypeEnum::Unknown)
				{
					type = e.type;
					break;
				}
			}
		}
		void GetNumberDefaultType(AttributeRecord& attr, PTDSBasicTypeEnum& type)
		{
			if (attr.float_str)
			{
				type = PTDSBasicTypeEnum::f64;
				return;
			}
			else if (attr.positive == false)
			{
				type = PTDSBasicTypeEnum::i64;
				return;
			}
			else
			{
				if (attr.hex_open)
				{
					PTDSBasicType::tsize i{ 0 };
					for (; i < attr.value.length(); ++i)
					{
						if (attr.value[i] != L'0')
						{
							break;
						}
					}
					// 0000abcdef len 10
					// 01234
					// len(10) - i(4) = 6 = len of valid
					// 0x ff ff ff ff ff ff ff ff
					if (attr.value.length() - i < 16)
					{
						type = PTDSBasicTypeEnum::i64;
						return;
					}
					else if (attr.value.length() - i == 16)
					{
						if (attr.value[i] >= L'8')
						{
							type = PTDSBasicTypeEnum::u64;
						}
						else
						{
							type = PTDSBasicTypeEnum::i64;
						}
						return;
					}
					else
					{
						type = PTDSBasicTypeEnum::u64;
						return;
					}
				}
				else if (attr.bin_open)
				{
					PTDSBasicType::tsize i{ 0 };
					for (; i < attr.value.length(); ++i)
					{
						if (attr.value[i] != L'0')
						{
							break;
						}
					}
					// 0000abcdef len 10
					// 01234
					// len(10) - i(4) = 6 = len of valid
					// 0x ff ff ff ff ff ff ff ff
					if (attr.value.length() - i < 64)
					{
						type = PTDSBasicTypeEnum::i64;
						return;
					}
					else
					{
						type = PTDSBasicTypeEnum::u64;
						return;
					}
				}
				else
				{
					constexpr PTDSBasicType::u64 max_i64{ ((~0LLU) >> 1) };
					static PTDSBasicType::str_utf16le max_i64_str{ ::std::to_wstring(max_i64) };
					if (attr.value > max_i64_str)
					{
						type = PTDSBasicTypeEnum::u64;
					}
					else
					{
						type = PTDSBasicTypeEnum::i64;
					}
					return;
				}
			}
			type = PTDSBasicTypeEnum::Unknown;
		}
		void AnNumberAttrSetToValue(AttributeRecordSet& attr_set, PTDSValueSet& dest)
		{
			switch (attr_set.type)
			{
			case PTDSBasicTypeEnum::Unknown:
				throw PTDSException(PTDSErrorCode::UnknownType, PTDSErrorMsg::UnknownType, attr_set.vset.begin()->line, attr_set.vset.begin()->position, L'\0');
				break;
			case PTDSBasicTypeEnum::bln:
			case PTDSBasicTypeEnum::cha:
			case PTDSBasicTypeEnum::str:
				throw PTDSException(PTDSErrorCode::WrongType, PTDSErrorMsg::WrongType, attr_set.vset.begin()->line, attr_set.vset.begin()->position, L'\0');
				break;
			default:
				break;
			}
			dest.t = attr_set.type;
			for (const auto& attr : attr_set.vset)
			{
				PTDSValueType v;
				AnNumberAttrToValue(attr, attr_set.type, v);
				dest.v.push_back(::std::move(v));
			}
		}
		void AnNumberAttrToValue(const AttributeRecord& attr, const PTDSBasicTypeEnum& target_type, PTDSValueType& dest)
		{
			if (target_type != PTDSBasicTypeEnum::f32 && target_type != PTDSBasicTypeEnum::f64)
			{
				if (attr.float_str == true)
				{
					throw PTDSException(PTDSErrorCode::WrongType, PTDSErrorMsg::WrongType, attr.line, attr.position, L'\0');
				}
			}
			if (attr.type != PTDSBasicTypeEnum::Unknown)
			{
				if (attr.type != target_type)
				{
					throw PTDSException(PTDSErrorCode::WrongType, PTDSErrorMsg::WrongType, attr.line, attr.position, L'\0');
				}
			}
			auto check_type = []
			(const PTDSBasicTypeEnum& target, const PTDSBasicTypeEnum& src, const PTDSBasicType::tsize& line, const PTDSBasicType::tsize& position) -> void
			{
				if (src != PTDSBasicTypeEnum::Unknown)
				{
					if (src != target)
					{
						throw PTDSException(PTDSErrorCode::WrongType, PTDSErrorMsg::WrongType, line, position, L'\0');
					}
				}
			};
			auto tran_u64 = []
			(const AttributeRecord& attr, PTDSBasicType::u64& dest) -> void
			{
				if (attr.positive == false)
				{
					throw PTDSException(PTDSErrorCode::WrongType, PTDSErrorMsg::WrongType, attr.line, attr.position, L'\0');
				}
				if (attr.float_str == true)
				{
					throw PTDSException(PTDSErrorCode::WrongType, PTDSErrorMsg::WrongType, attr.line, attr.position, L'\0');
				}
				if (attr.bin_open == true)
				{
					if (AnNumberBStringToU64(attr, dest) == false)
					{
						throw PTDSException(PTDSErrorCode::NumberOutOfRange, PTDSErrorMsg::NumberOutOfRange, attr.line, attr.position, L'\0');
					}
				}
				else if (attr.hex_open == true)
				{
					if (AnNumberHStringToU64(attr, dest) == false)
					{
						throw PTDSException(PTDSErrorCode::NumberOutOfRange, PTDSErrorMsg::NumberOutOfRange, attr.line, attr.position, L'\0');
					}
				}
				else
				{
					if (AnNumberDStringToU64(attr, dest) == false)
					{
						throw PTDSException(PTDSErrorCode::NumberOutOfRange, PTDSErrorMsg::NumberOutOfRange, attr.line, attr.position, L'\0');
					}
				}
			};
			auto tran_u64_for_i = [] // 这个版本不检查符号是否为负
			(const AttributeRecord& attr, PTDSBasicType::u64& dest) -> void
			{
				if (attr.float_str == true)
				{
					throw PTDSException(PTDSErrorCode::WrongType, PTDSErrorMsg::WrongType, attr.line, attr.position, L'\0');
				}
				if (attr.bin_open == true)
				{
					if (AnNumberBStringToU64(attr, dest) == false)
					{
						throw PTDSException(PTDSErrorCode::NumberOutOfRange, PTDSErrorMsg::NumberOutOfRange, attr.line, attr.position, L'\0');
					}
				}
				else if (attr.hex_open == true)
				{
					if (AnNumberHStringToU64(attr, dest) == false)
					{
						throw PTDSException(PTDSErrorCode::NumberOutOfRange, PTDSErrorMsg::NumberOutOfRange, attr.line, attr.position, L'\0');
					}
				}
				else
				{
					if (AnNumberDStringToU64(attr, dest) == false)
					{
						throw PTDSException(PTDSErrorCode::NumberOutOfRange, PTDSErrorMsg::NumberOutOfRange, attr.line, attr.position, L'\0');
					}
				}
			};
			PTDSBasicType::u64 cmpa{ 0 };
			switch (target_type)
			{
			case PTDSBasicTypeEnum::u8:
				check_type(target_type, attr.type, attr.line, attr.position);
				tran_u64(attr, dest.u64);
				if (dest.u64 > 0xff)
				{
					throw PTDSException(PTDSErrorCode::NumberOutOfRange, PTDSErrorMsg::NumberOutOfRange, attr.line, attr.position, L'\0');
				}
				dest.u8 = static_cast<PTDSBasicType::u8>(dest.u64);
				dest.str = attr.sign + attr.format + attr.value;
				break;
			case PTDSBasicTypeEnum::u16:
				check_type(target_type, attr.type, attr.line, attr.position);
				tran_u64(attr, dest.u64);
				if (dest.u64 > 0xffff)
				{
					throw PTDSException(PTDSErrorCode::NumberOutOfRange, PTDSErrorMsg::NumberOutOfRange, attr.line, attr.position, L'\0');
				}
				dest.u16 = static_cast<PTDSBasicType::u16>(dest.u64);
				dest.str = attr.sign + attr.format + attr.value;
				break;
			case PTDSBasicTypeEnum::u32:
				check_type(target_type, attr.type, attr.line, attr.position);
				tran_u64(attr, dest.u64);
				if (dest.u64 > 0xffffffff)
				{
					throw PTDSException(PTDSErrorCode::NumberOutOfRange, PTDSErrorMsg::NumberOutOfRange, attr.line, attr.position, L'\0');
				}
				dest.u32 = static_cast<PTDSBasicType::u32>(dest.u64);
				dest.str = attr.sign + attr.format + attr.value;
				break;
			case PTDSBasicTypeEnum::u64:
				check_type(target_type, attr.type, attr.line, attr.position);
				tran_u64(attr, dest.u64);
				dest.str = attr.sign + attr.format + attr.value;
				break;
			case PTDSBasicTypeEnum::i8:
				check_type(target_type, attr.type, attr.line, attr.position);
				tran_u64_for_i(attr, dest.u64);
				if (attr.positive == true)
				{
					if (dest.u64 > 0x7f)
					{
						throw PTDSException(PTDSErrorCode::NumberOutOfRange, PTDSErrorMsg::NumberOutOfRange, attr.line, attr.position, L'\0');
					}
					dest.i8 = static_cast<PTDSBasicType::i8>(dest.u64);
				}
				else
				{
					if (dest.u64 > 0x80)
					{
						throw PTDSException(PTDSErrorCode::NumberOutOfRange, PTDSErrorMsg::NumberOutOfRange, attr.line, attr.position, L'\0');
					}
					dest.i8 = -static_cast<PTDSBasicType::i8>(dest.u64);
				}
				dest.str = attr.sign + attr.format + attr.value;
				break;
			case PTDSBasicTypeEnum::i16:
				check_type(target_type, attr.type, attr.line, attr.position);
				tran_u64_for_i(attr, dest.u64);
				if (attr.positive == true)
				{
					if (dest.u64 > 0x7fff)
					{
						throw PTDSException(PTDSErrorCode::NumberOutOfRange, PTDSErrorMsg::NumberOutOfRange, attr.line, attr.position, L'\0');
					}
					dest.i16 = static_cast<PTDSBasicType::i16>(dest.u64);
				}
				else
				{
					if (dest.u64 > 0x8000)
					{
						throw PTDSException(PTDSErrorCode::NumberOutOfRange, PTDSErrorMsg::NumberOutOfRange, attr.line, attr.position, L'\0');
					}
					dest.i16 = -static_cast<PTDSBasicType::i16>(dest.u64);
				}
				dest.str = attr.sign + attr.format + attr.value;
				break;
			case PTDSBasicTypeEnum::i32:
				check_type(target_type, attr.type, attr.line, attr.position);
				tran_u64_for_i(attr, dest.u64);
				if (attr.positive == true)
				{
					if (dest.u64 > 0x7fffffff)
					{
						throw PTDSException(PTDSErrorCode::NumberOutOfRange, PTDSErrorMsg::NumberOutOfRange, attr.line, attr.position, L'\0');
					}
					dest.i32 = static_cast<PTDSBasicType::i32>(dest.u64);
				}
				else
				{
					if (dest.u64 > 0x80000000)
					{
						throw PTDSException(PTDSErrorCode::NumberOutOfRange, PTDSErrorMsg::NumberOutOfRange, attr.line, attr.position, L'\0');
					}
					dest.i32 = -static_cast<PTDSBasicType::i32>(dest.u64);
				}
				dest.str = attr.sign + attr.format + attr.value;
				break;
			case PTDSBasicTypeEnum::i64:
				check_type(target_type, attr.type, attr.line, attr.position);
				tran_u64_for_i(attr, dest.u64);
				if (attr.positive == true)
				{
					if (dest.u64 > 0x7fffffffffffffff)
					{
						throw PTDSException(PTDSErrorCode::NumberOutOfRange, PTDSErrorMsg::NumberOutOfRange, attr.line, attr.position, L'\0');
					}
					dest.i64 = static_cast<PTDSBasicType::i64>(dest.u64);
				}
				else
				{
					if (dest.u64 > 0x8000000000000000)
					{
						throw PTDSException(PTDSErrorCode::NumberOutOfRange, PTDSErrorMsg::NumberOutOfRange, attr.line, attr.position, L'\0');
					}
					dest.i64 = -static_cast<PTDSBasicType::i64>(dest.u64);
				}
				dest.str = attr.sign + attr.format + attr.value;
				break;
			case PTDSBasicTypeEnum::f32:
				if (attr.bin_open == true || attr.hex_open == true)
				{
					throw PTDSException(PTDSErrorCode::WrongType, PTDSErrorMsg::WrongType, attr.line, attr.position, L'\0');
				}
				AnNumberDStringToF32(attr, dest.f32);
				dest.str = attr.sign + attr.value;
				break;
			case PTDSBasicTypeEnum::f64:
				if (attr.bin_open == true || attr.hex_open == true)
				{
					throw PTDSException(PTDSErrorCode::WrongType, PTDSErrorMsg::WrongType, attr.line, attr.position, L'\0');
				}
				AnNumberDStringToF64(attr, dest.f64);
				dest.str = attr.sign + attr.value;
				break;
			default:
				break;
			}
		}
		bool AnNumberDStringToU64(const AttributeRecord& attr, PTDSBasicType::u64& dest)
		{
			try
			{
				dest = ::std::stoull(attr.value);
			}
			catch (::std::out_of_range&)
			{
				return false;
			}
			return true;
		}
		bool AnNumberDStringToF32(const AttributeRecord& attr, PTDSBasicType::f32& dest)
		{
			try
			{
				dest = ::std::stof(attr.value);
				if (attr.positive == false)
				{
					dest = -dest;
				}
			}
			catch (::std::invalid_argument&)
			{
				return false;
			}
			return true;
		}
		bool AnNumberDStringToF64(const AttributeRecord& attr, PTDSBasicType::f64& dest)
		{
			try
			{
				dest = ::std::stod(attr.value);
				if (attr.positive == false)
				{
					dest = -dest;
				}
			}
			catch (::std::invalid_argument&)
			{
				return false;
			}
			return true;
		}
		bool AnNumberBStringToU64(const AttributeRecord& attr, PTDSBasicType::u64& dest)
		{
			unsigned long long i{ 1 };
			unsigned long long b{ 1 };
			auto& num_str{ attr.value };
			for (auto it = num_str.rbegin(); it != num_str.rend(); ++it)
			{
				if (*it == L'1')
				{
					if (b > 64)
					{
						return false;
					}
					dest |= i;
				}
				i *= 2;
				++b;
			}
			return true;
		}
		bool AnNumberHStringToU64(const AttributeRecord& attr, PTDSBasicType::u64& dest)
		{
			unsigned long long i{ 0 };
			unsigned long long offset{ 0 };
			auto& num_str{ attr.value };
			for (auto it = num_str.rbegin(); it != num_str.rend(); ++it)
			{
				i = 0;
				if ((*it) >= L'1' && (*it) <= L'9')
				{
					i = static_cast<PTDSBasicType::u64>(*it) - L'0';
				}
				else if ((*it) >= L'A' && (*it) <= L'F')
				{
					i = static_cast<PTDSBasicType::u64>(*it) - L'A' + 10;
				}
				else if ((*it) >= L'a' && (*it) <= L'f')
				{
					i = static_cast<PTDSBasicType::u64>(*it) - L'a' + 10;
				}
				if (i > 0)
				{
					if (offset > 60)
					{
						return false;
					}
					dest |= (i << offset);
				}
				offset += 4;
			}
			return true;
		}
		void AnBooleanAttrSetToValue(AttributeRecordSet& attr_set, PTDSValueSet& dest)
		{
			if (attr_set.type != PTDSBasicTypeEnum::bln)
			{
				throw PTDSException(PTDSErrorCode::WrongType, PTDSErrorMsg::WrongType, attr_set.vset.begin()->line, attr_set.vset.begin()->position, L'\0');
			}
			dest.t = attr_set.type;
			for (const auto& attr : attr_set.vset)
			{
				PTDSValueType v;
				AnBooleanAttrToValue(attr, attr_set.type, v);
				dest.v.push_back(::std::move(v));
			}
		}
		void AnBooleanAttrToValue(const AttributeRecord& attr, const PTDSBasicTypeEnum& target_type, PTDSValueType& dest)
		{
			if (attr.type != PTDSBasicTypeEnum::Unknown && attr.type != PTDSBasicTypeEnum::bln)
			{
				throw PTDSException(PTDSErrorCode::WrongType, PTDSErrorMsg::WrongType, attr.line, attr.position, L'\0');
			}
			if (attr.float_str == true)
			{
				throw PTDSException(PTDSErrorCode::WrongType, PTDSErrorMsg::WrongType, attr.line, attr.position, L'\0');
			}
			switch (attr.value[0])
			{
			case L'T':
			case L't':
				dest.bln = true;
				dest.str = attr.value;
				return;
				break;
			case L'F':
			case L'f':
				dest.bln = false;
				dest.str = attr.value;
				return;
				break;
			default:
				break;
			}
			PTDSBasicType::u64 temp{ 0 };
			if (attr.bin_open == true)
			{
				if (AnNumberBStringToU64(attr, temp) == false)
				{
					throw PTDSException(PTDSErrorCode::NumberOutOfRange, PTDSErrorMsg::NumberOutOfRange, attr.line, attr.position, L'\0');
				}
			}
			else if (attr.hex_open == true)
			{
				if (AnNumberHStringToU64(attr, temp) == false)
				{
					throw PTDSException(PTDSErrorCode::NumberOutOfRange, PTDSErrorMsg::NumberOutOfRange, attr.line, attr.position, L'\0');
				}
			}
			else
			{
				if (AnNumberDStringToU64(attr, temp) == false)
				{
					throw PTDSException(PTDSErrorCode::NumberOutOfRange, PTDSErrorMsg::NumberOutOfRange, attr.line, attr.position, L'\0');
				}
			}
			if (temp == 0)
			{
				dest.bln = false;
			}
			else
			{
				dest.bln = true;
			}
			dest.str = attr.sign + attr.format + attr.value;
		}
		void AnStringAttrSetToValue(AttributeRecordSet& attr_set, PTDSValueSet& dest)
		{
			if (attr_set.type != PTDSBasicTypeEnum::str && attr_set.type != PTDSBasicTypeEnum::cha)
			{
				throw PTDSException(PTDSErrorCode::WrongType, PTDSErrorMsg::WrongType, attr_set.vset.begin()->line, attr_set.vset.begin()->position, L'\0');
			}
			dest.t = attr_set.type;
			switch (attr_set.type)
			{
			case PTDSBasicTypeEnum::str:
				for (const auto& attr : attr_set.vset)
				{
					PTDSValueType v;
					AnStringAttrToValue(attr, attr_set.type, v);
					dest.v.push_back(::std::move(v));
				}
				break;
			case PTDSBasicTypeEnum::cha:
				for (const auto& attr : attr_set.vset)
				{
					PTDSValueType v;
					AnStringAttrToValue(attr, attr_set.type, v);
					if (v.str.length() != 1)
					{
						throw PTDSException(PTDSErrorCode::WrongType, PTDSErrorMsg::WrongType, attr_set.vset.begin()->line, attr_set.vset.begin()->position, L'\0');
					}
					v.cha = v.str[0];
					dest.v.push_back(::std::move(v));
				}
				break;
			default:
				throw PTDSException(PTDSErrorCode::WrongType, PTDSErrorMsg::WrongType, attr_set.vset.begin()->line, attr_set.vset.begin()->position, L'\0');
				break;
			}
		}
		void AnStringAttrToValue(const AttributeRecord& attr, const PTDSBasicTypeEnum& target_type, PTDSValueType& dest)
		{
			if (attr.type != PTDSBasicTypeEnum::Unknown)
			{
				if (attr.type != target_type)
				{
					throw PTDSException(PTDSErrorCode::WrongType, PTDSErrorMsg::WrongType, attr.line, attr.position, L'\0');
				}
			}
			if (target_type == PTDSBasicTypeEnum::cha)
			{
				if (attr.value.size() != 1)
				{
					throw PTDSException(PTDSErrorCode::WrongType, PTDSErrorMsg::WrongType, attr.line, attr.position, L'\0');
				}
				dest.cha = attr.value[0];
				dest.str = attr.value;
			}
			else if (target_type == PTDSBasicTypeEnum::str)
			{
				dest.str = attr.value;
			}
			else
			{
				throw PTDSException(PTDSErrorCode::WrongType, PTDSErrorMsg::WrongType, attr.line, attr.position, L'\0');
			}
		}
	}
}