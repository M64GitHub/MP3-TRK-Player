#include "m64_media_file.hpp"
#include <stdlib.h>
#include <string.h>

m64_media_filelist::m64_media_filelist() {
  count = 0;
  files = 0;
}

m64_media_filelist::m64_media_filelist(int size) { init(size); }

void m64_media_filelist::init(int size) {
  count = 0;
  max = size;
  files = (m64_media_file *)malloc(max * sizeof(m64_media_file));
}

void m64_media_filelist::append(m64_media_file *f) {
  if (count && (count == max))
    return;

  strncpy(files[count].name, f->name, ENTRY_NAME_MAX_LEN);
  files[count].size = f->size;
  files[count].type = f->type;

  count++;
}
