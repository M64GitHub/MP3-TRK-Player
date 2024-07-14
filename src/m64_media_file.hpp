#ifndef M64_MEDIAFILE_H
#define M64_MEDIAFILE_H

#define FILE_TYPE_MP3 1
#define FILE_TYPE_IT 2
#define FILE_TYPE_DIRECTORY 4

#define ENTRY_NAME_MAX_LEN 64
#define ENTRY_PATH_MAX_LEN 128

class m64_media_file {
public:
  char name[ENTRY_NAME_MAX_LEN];
  unsigned int size;
  char type;
};

class m64_media_filelist {
public:
  m64_media_filelist();
  m64_media_filelist(int size);

  void init(int size); // malloc
  void append(m64_media_file *f);

  int max = 0;
  int count = -1;
  m64_media_file *files = 0;
};

#endif
