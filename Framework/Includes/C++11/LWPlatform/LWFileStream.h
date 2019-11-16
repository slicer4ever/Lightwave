#ifndef LWFILESTREAM_H
#define LWFILESTREAM_H
#include "LWCore/LWTypes.h"
#include "LWCore/LWText.h"
#include <cstdio>
/*! \addtogroup LWCore
	@{
*/
/*! \brief A file stream object for reading and writing file i/o. */

class LWFileStream{
public:
	static const uint32_t ReadMode   = 0x1; /*!< \brief Marks the file as a read file. */
	static const uint32_t WriteMode  = 0x2; /*!< \brief Marks the file as a write file. */
	static const uint32_t AppendMode = 0x4; /*!< \brief Marks the file as an appended file. */
	static const uint32_t BinaryMode = 0x8; /*!< \brief Marks the file as being in binary mode. */
	static const uint32_t AssetMode = 0x10; /*!< \brief internally used to indicate using platform specific api to access and read/write files. */


	static const uint32_t Game = 0; /*!< \brief the local game folder id. save all content here needed to be preserved that is not user data. */
	static const uint32_t Fonts = 1; /*!< \brief the system fonts folder id. */ 
	static const uint32_t App = 2; /*!< \brief the read only content folder(or asset folder packed with the apk)*/
	static const uint32_t User = 3; /*!< \brief the system's user folder id. save data that is use specific here.*/

	static const uint8_t SeekCurrent = 0; /*!< \brief Seek flag from the current position. */
	static const uint8_t SeekStart = 1; /*!< \brief Seek flag from the beginning of the file. */
	static const uint8_t SeekEnd = 2; /*!< \brief Seek flag from the end of the file. */

	static const uint8_t AssetToken = '!'; /*!< \brief token placed at beginning of file path to indicate asset mode is to be used. */

	/*! \brief returns the system absolute path to the specified file.
		\param FilePath the relative file path from the current active folder.
		\param Buffer buffer may be null, in which case nothing is written to buffer, and bufferLen is ignored.
		\param BufferLen the size of the buffer to receive the absolute file path.
		\return the length of the expected buffer size to receive the fullpath(0 if failure occurred).
	*/
	static uint32_t MakeAbsolutePath(const LWText &FilePath, char *Buffer, uint32_t BufferLen);

	/*! \brief returns the directory path of the filepath.
		\param FilePath the filepath to parse for a directory.
		\param Buffer buffer may be null, in which case nothing is written to buffer, and bufferLen is ignored.
		\param BufferLen the size of the buffer to receive the directory name.
		\return the length of the expected buffer size to receive the directory path.
	*/
	static uint32_t MakeDirectoryPath(const LWText &FilePath, char *Buffer, uint32_t BufferLen);

    /*!< \brief returns the file name from the directory path.
        \param FilePath the filepath to parse for a filename.
        \param Buffer buffer may be null, in which case nothing is written to buffer, and bufferLen is ignored.
        \param BufferLen the size of the buffer to receive the filename.
        \return the length of the expected buffer size to receive the filename.
     */
    static uint32_t MakeFileName(const LWText &FilePath, char *Buffer, uint32_t BufferLen);
    
	/*! \brief returns a relative path between two file path's. 
		\param From the directory or file to navigate from.
		\param To the directory or file to navigate to.
		\param Buffer buffer may be null, in which case nothing is written to buffer, and bufferLen is ignored.
		\param BufferLen the size of the buffer to receive the relative path navigating From to To.
		\return the length of the expected buffer size to receive the relative path.
	*/
	static uint32_t MakeRelativePath(const LWText &From, const LWText &To, char *Buffer, uint32_t BufferLen);

	/*! \brief returns rather the path is absolute or not. */
	static bool PathIsAbsolute(const LWText &FilePath);

	/*! \brief writes into buffer the extension of the file. */
	static uint32_t GetExtension(const LWText &FilePath, char *Buffer, uint32_t BufferLen);

	/*! \brief returns rather the file has the extension requested.
		\note that this function is not case sensitive for ascii characters.
	*/
	static bool IsExtension(const LWText &FilePath, const char *Extension);

	/*! \brief returns which extension the filepath is apart of. takes a variable list of const char * objects
		\return the index of the extension, of 0xFFFFFFFF if no extension was found.
		\note that this function is not case sensitive for ascii characters.
	*/
	static uint32_t IsExtensions(const LWText &FilePath, uint32_t ExtCount, ...);

	/*! \brief Returns the system paths for the specified directors. 
		\return rather this function succeeded or failed at finding the specified folder.
	*/
	static bool GetFolderPath(uint32_t FolderID, char *Buffer, uint32_t BufferLen);
    
	/*! \brief writes into buffer a concated extension onto the file, if the file does not already have the extension.
		\return the number of characters for the buffer to be.
	*/
	static uint32_t ConcatExtension(const LWText &FilePath, const LWText &Extension, char *Buffer, uint32_t BufferLen);

	/*! \brief Parses the input file path to generate a new correct file path.
		\param FilePath the original FilePath object to be transformed.
		\param Buffer the buffer to receive the resulted transformation.
		\param BufferLen the length of the buffer.
		\param ExistingStream the existing file stream which a relative path is to be built off of.
		\return true if parsing was successful, false if parsing failed.
		\note there are several prefixs that can be used file paths to simplify obtaining the system's file paths.
			  "Game:Path" will result in a file path relative to the directory the app was started in.
			  "Fonts:Path" will result in a file path relative to the systems font directory. 
			  "App:Path" will result in a file path relative to the systems program data folder that can be used for global user storage.
			  "User" will result in a file path relative to the systems per-user storage location.
			  If ExistingStream is not null, then the file path supplied is assumed to be relative to the existing stream's file.
	*/
	static bool ParsePath(const LWText &FilePath, char *Buffer, uint32_t BufferLen, const LWFileStream *ExistingStream = nullptr);

	/*! \brief opens a file stream for use, Allocator is used to create an LWText object passed to the resulting file stream. */
	static bool OpenStream(LWFileStream &Result, const LWText &FilePath, uint32_t Flag, LWAllocator &Allocator, const LWFileStream *ExistingStream = nullptr);

	/*! \overload bool OpenStream(LWFileStream &, const char *, uint8_t, LWAllocator &) */
	static bool OpenStream(LWFileStream &Result, const char *FilePath, uint32_t Flag, LWAllocator &Allocator, const LWFileStream *ExistingStream = nullptr);

	/*! \brief opens a file stream for use, where file path is a variable formated utf-8 string. Allocator is used to create an LWText object passed to the resulting file stream. */
	static bool OpenStreamf(LWFileStream &Result, const uint8_t *FilePath, uint32_t Flag, LWAllocator &Allocator, const LWFileStream *ExistingStream, ...);

	/*! \overload bool OpenStreamf(LWFileStream &, const char *, uint8_t Flag, LWAllocator &, ...); */
	static bool OpenStreamf(LWFileStream &Result, const char *FilePath, uint32_t Flag, LWAllocator &Allocator, const LWFileStream *ExistingStream, ...);

	/*!< \brief handles opening and closing a file resource to check that it exists. 
		 \return true on success, false on failure.
	*/
	static bool Exists(const LWText &Filepath);

	/*!< \brief moves a file on the file system to another location. (note: misspelling is on purpose due to name collisions on certain platforms.)
		 \return true on success, false on failure.
	*/
	static bool MovFile(const LWText &SrcFilepath, const LWText &DstFilepath);

	/*!< \brief deletes a file on the file system.
		 \return true on success, false on failure.
	*/
	static bool DelFile(const LWText &Filepath);


	/*! \brief set's and move's the data from other to this object. */
	LWFileStream &operator = (LWFileStream &&Other);

	/*! \brief returns one byte from the file object. */
	uint8_t ReadByte(void);
	
	/*! \brief Read's text from the file upto BufferLen. 
		\return the number of characters actually read.
	*/
	uint32_t ReadText(uint8_t *Buffer, uint32_t BufferLen);

	/*! \overload uint32_t ReadText(char *, uint32_t) */
	uint32_t ReadText(char *Buffer, uint32_t BufferLen);

	/*! \brief Read's a single line of text from the file.
		\return the number of characters actually read.
	*/
	uint32_t ReadTextLine(uint8_t *Buffer, uint32_t BufferLen);

	/*! \overload uint32_t ReadTextLine(char *, uint32_t) */
	uint32_t ReadTextLine(char *Buffer, uint32_t BufferLen);

	/*! \brief Read's data from the file upto Len.
		\return number of bytes read back.
	*/
	uint32_t Read(uint8_t *Buffer, uint32_t Len);

	/*! \overload uint32_t Read(char *, uint32_t) */
	uint32_t Read(char *Buffer, uint32_t Len);

	/*! \brief Write's data to the file.
		\return number of bytes written to the file.
	*/
	uint32_t Write(const uint8_t *Buffer, uint32_t Len);

	/*! \overload Write(const char * uint32_t) */
	uint32_t Write(const char *Buffer, uint32_t Len);

	/*! \brief write's a single byte to the file. */
	LWFileStream &WriteByte(uint8_t Byte);

	/*! \brief write's text into the file.
		\return the number of bytes written.
	*/
	uint32_t WriteText(const LWText &Text);

	/*! \brief write's variable text into the file. */
	uint32_t WriteTextf(const char *Format, ...);

	/*! \brief Seek's the position of the file stream. */
	LWFileStream &Seek(int32_t Offset, uint8_t SeekFlag = SeekCurrent);

	/*! \brief closes the internal file stream without destroying the the local filestream object. */
	LWFileStream &Finished(void);

	/*! \brief returns the length of the file. */
	uint32_t Length(void) const;

	/*! \brief returns the current position in the file. */
	uint32_t GetPosition(void) const;

	/*! \brief returns rather the internal file object is at the end of file or not. */
	bool EndOfStream(void) const;

	/*! \brief returns the underlying file object associated with this filestream. */
	FILE *GetFileObject(void);

	/*!< \brief returns the flags for the file. */
	uint32_t GetFlag(void) const;

	/*! \brief returns the file path to the file. */
	const LWText &GetFilePath(void) const;

	/*! \brief returns the directory path to the file. */
	const LWText &GetDirectoryPath(void) const;

	/*!< \brief returns the time the file was created. */
	uint64_t GetCreatedTime(void) const;

	/*!< \brief returns the time the file was last modified. */
	uint64_t GetModifiedTime(void) const;

	/*!< \brief returns the time the file was last accessed. */
	uint64_t GetAccessedTime(void) const;

	/*! \brief creates an empty file object. */
	LWFileStream();

	/*! \brief move constructor, nullify's the original file object. */
	LWFileStream(LWFileStream &&Other);

	/*! \brief constructs a file stream object. */
	LWFileStream(FILE *FileObject, const LWText &FilePath, uint32_t Flags, LWAllocator &Allocator);

	/*! \brief destructs a file stream object. */
	~LWFileStream();
private:
	FILE *m_FileObject; //The underlying file handle.
	LWText m_FilePath; //String representation of the file's location.
	LWText m_DirPath; //String representation of the directory the file is located in.
	uint64_t m_CreateTime; //Time when the file was created.
	uint64_t m_ModifiedTime; //Time when the file was last modified.
	uint64_t m_AccessedTime; //Time when the file was last accessed.
	uint32_t m_Length; //Size of the file in bytes.
	uint32_t m_Flag; //Internal flags for accessing the file.
};

/*! @} */
#endif