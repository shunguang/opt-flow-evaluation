#ifndef _APP_UTIL_H_
#define _APP_UTIL_H_

#include "DataTypes.h"
#include <sys/resource.h>

#ifdef _WINDOWS
#include <io.h> // For access().
#include <direct.h>
#define GetCurrentDir _getcwd
#define appAccess _access
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#define appAccess access
#endif

namespace app
{
	uint32_t getFileNameList(const std::string &dirName, const std::string &ext, std::vector<std::string> &vFileNames);
	uint32_t getFileNameList2(const std::string &dirName, const std::string &pattern, std::vector<std::string> &vFileNames);
	void getExtName(const std::string &fileName, std::string &ext);
	bool folderExists(const std::string &strPath);
	bool fileExists(const std::string &name);

	bool mkDumpDirs(const std::string &inSeqFileName, const std::string &dumpPathRoot, std::string &dumpPath4Debug, std::string &dumpPath4Customer);
	bool findSeqName(const std::string &inSeqFileName, std::string &seqName);
	std::string getCurDir();
	void deleteFilesInFolder(const std::string &folderPath);

	void createDir(const std::string &p);

	//fPath="c:/temp/f1.txt" ---> head = "c:/temp/f1", ext="txt"
	void splitExt(const std::string &fPath, std::string &head, std::string &ext);
	//fPath="c:/temp/f1.txt" ---> fDir = "c:/temp", fName="f1.txt"
	void splitFolder(const std::string &fPath, std::string &fDir, std::string &fName);

	bool isVideoFile(const std::string &filePath);
	bool isImgeFile(const std::string &filePath);
	uint64_t getAvailableDiskSpaceInByte(const std::string &folderPath);

	uint32_t checkTimeOut(const boost::posix_time::ptime &start, const uint32_t thdInMillisec);
	uint32_t timeIntervalMillisec(const boost::posix_time::ptime &start);
	uint32_t timeIntervalMillisec(const boost::posix_time::ptime &start, const boost::posix_time::ptime &end);
	void     timeIntervalMillisec( const rusage &ru1, const rusage &ru2, int64_t &dt_usr_cpu_us, int64_t &dt_sys_cpu_us );

	std::string getFileName(const std::string &tag, const std::string &ext, const int fn, const char* fmt );
	std::string getFileName2(const std::string &tag, const std::string &ext, const int fn, const char* fmt );


	//-----------------------------
	struct PcktBuffer
	{
		PcktBuffer(const size_t sz, const size_t initVal = 0)
			: length(sz)
		{
			buffer = new char[sz];
			//intialized as 'x's
			for (size_t i = 0; i < sz; ++i)
				buffer[i] = 'x';

			//add the str of <initVal> at begining for dubug purpose
			std::string s = std::to_string(initVal);
			size_t L = (sz > s.length()) ? s.length() : sz;
			for (size_t i = 0; i < L; i++)
			{
				buffer[i] = s[i];
			}
		}

		~PcktBuffer()
		{
			if (buffer)
			{
				delete[] buffer;
			}
			length = 0;
		}

		char *buffer{nullptr};
		int length{0};
	};

}

#endif
