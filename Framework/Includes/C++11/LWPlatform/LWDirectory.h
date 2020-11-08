#ifndef LWDIRECTORY_H
#define LWDIRECTORY_H
#include <LWCore/LWTypes.h>
#include <LWCore/LWAllocator.h>
#include <LWCore/LWUnicode.h>
#include <LWPlatform/LWTypes.h>
#include <LWPlatform/LWPlatform.h>

/*! \addtogroup LWPlatform LWPlatform
	@{
*/
/*! \brief the maximum length an individual file can be. */
#define LWMAXFILENAME_SIZE 256

/*! \brief individual file/directory object, which contains supplementary information about the file.*/

struct LWFile{
	char8_t m_Name[LWMAXFILENAME_SIZE]; /*!< \brief 256 byte buffer to contain the associated filename of the file in question. */
	uint64_t m_Size; /*!< \brief holds the size of the file, for directorys this will be 0, note that being 0 is not a reliable way to check if the file is actually a directory, be sure to check the flag. */
	uint32_t m_Flag; /*!< \brief supplementary flag that represents the file status, see LWDirectory for a list of possible flags this file can contain, and there supplimentary meaning. */

	/*!< \brief returns if the file is a directory path. */
	bool isDirectory(void) const;

	/*!< \brief returns if the file is suppose to be hidden from the user normally. */
	bool isHidden(void) const;

	/*!< \brief returns if the file is readable. */
	bool isReadable(void) const;

	/*!< \brief returns if he file is writeable. */
	bool isWriteable(void) const;

	/*!< \brief constructor for LWFile. */
	LWFile(const LWUTF8Iterator &Name, uint64_t Size, uint32_t Flag);

	/*!< \brief default constructor for LWFile. */
	LWFile() = default;
};
	
/*! \brief LWDirectory cache's upto LWMAXFILES files/directorys at a time, if the application expects to encounter more than 256 files, then it is recommended to use a different directory managing class, or recompiling this module with a higher limit.*/
class LWDirectory{
public:
	static const uint32_t Directory = 0x1; /*!< \brief flag which represents the file is actually a directory. */
	static const uint32_t Hidden = 0x2; /*!< \brief flag which represents the file is marked as hidden by the os. */
	static const uint32_t CanRead = 0x4; /*!< \brief flag which represents that the file can be read from. */
	static const uint32_t CanWrite = 0x8; /*!< \brief flag which represents that the file can be written to. */

	/*! \brief checks if the actual directory exists or not. */
	static bool DirExists(const LWUTF8Iterator &DirectoryPath, const LWFileStream *ExistingStream = nullptr);

	/*! \brief opens a directory path if possible, if failure occurs, false is returned. */
	static bool OpenDir(LWDirectory &DirObject, const LWUTF8Iterator &DirectoryPath, LWAllocator &Allocator, const LWFileStream *ExistingStream = nullptr);

	/*! \brief Creates the specified directory and any parent directorys if they don't already exist.
		\param DirectoryPath the path to the directory to be created.
		\param ExistingStream any existing stream that should be taken into account when creating the path.
	*/
	static bool CreateDir(const LWUTF8Iterator &DirectoryPath, const LWFileStream *ExistingStream = nullptr);

	/*!, \brief parse's directory path like filestream::ParseBuffer, and attempts to ensure the last char to Buffer is a '/'. */
	static uint32_t ParsePath(const LWUTF8Iterator &DirectoryPath, char8_t *Buffer, uint32_t BufferLen, const LWFileStream *ExistingStream = nullptr);

	/*! \brief move operator. */
	LWDirectory &operator = (LWDirectory &&Other);

	/*! \brief searches the directory for the specified file or folder. returns null if not found. */
	const LWFile *FindFile(const LWUTF8Iterator &FileName) const;

	/*! \brief returns the file/folder at the specified index, Index is valid from FileCount+FolderCount length, no guarantee on order of files/folders, may return null if out of bounds. */
	const LWFile *GetFile(uint32_t i) const;

	/*!< \brief returns the total number of folder+regular files. */
	uint32_t GetTotalFiles(void) const;

	/*! \brief returns the total number of non-directory files contained in the directory. */
	uint32_t GetFileCount(void) const;

	/*! \brief returns the total number of folders contained in the directory. */
	uint32_t GetFolderCount(void) const;

	/*! \brief returns the directory path of this directory. */
	const LWUTF8 &GetDirectoryPath(void) const;

	/*! \brief constructs a default move constructor. */
	LWDirectory(LWDirectory &&Other);

	/*! \brief constructs an LWDirectory object. */
	LWDirectory(const LWUTF8Iterator &DirectoryPath, LWAllocator &Allocator);

	LWDirectory() = default;
private:

	/*!< \brief platform specific cerate's the specified directory at path. */
	static bool CreateDir(const LWUTF8Iterator &DirectoryPath, bool isParentDir);

	bool PushFile(const LWFile &F);

	LWUTF8 m_DirectoryPath;
	std::vector<LWFile> m_FileList;
	uint32_t m_FolderCount = 0;
	uint32_t m_FileCount = 0;
	
};
/*! @} */
#endif