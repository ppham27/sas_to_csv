#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef unsigned char byte;

/* lengths are measured in bytes */

/* header constants */
const size_t HEADER_LENGTH = 1024;
const size_t MAGIC_NUMBER_LENGTH = 32;
const byte REAL_MAGIC_NUMBER[] = {0x00,0x00,0x00,0x00,
                                  0x00,0x00,0x00,0x00,
                                  0x00,0x00,0x00,0x00,
                                  0xc2,0xea,0x81,0x60,
                                  0xb3,0x14,0x11,0xcf,
                                  0xbd,0x92,0x08,0x00,
                                  0x09,0xc7,0x31,0x8c,
                                  0x18,0x1f,0x10,0x11};

/* alignment */
const size_t ALIGN_OFFSET = 35;
const size_t ALIGN_LENGTH = 2;
const byte ALIGN_64[] = {0x33,0x33};
const size_t ALIGN_64_OFFSET = 4;
const byte ALIGN_32[] = {0x32,0x22};
const size_t ALIGN_32_OFFSET = 0;

const size_t SAS_LABEL_OFFSET = 84;
const size_t SAS_LABEL_LENGTH = 8;

const size_t DATASET_NAME_OFFSET = 92;
const size_t DATASET_NAME_LENGTH = 64;

const size_t FILE_TYPE_OFFSET = 156;
const size_t FILE_TYPE_LENGTH = 8;

const size_t TIMESTAMP_OFFSET = 164;

const size_t PAGE_SIZE_OFFSET = 200;
const size_t PAGE_COUNT_OFFSET = 204;

const size_t SAS_RELEASE_OFFSET = 216;
const size_t SAS_RELEASE_LENGTH = 8;

const size_t SAS_HOST_OFFSET = 224;
const size_t SAS_HOST_LENGTH = 8;

/* subheader constants */
/* subheader signatures */
const size_t SUBH_SIG_LENGTH = 4;
const byte SUBH_ROWSIZE[] = {0xf7,0xf7,0xf7,0xf7};
const byte SUBH_COLSIZE[] = {0xf6,0xf6,0xf6,0xf6};
const byte SUBH_COLTEXT[] = {0xfd,0xff,0xff,0xff};
const byte SUBH_COLATTR[] = {0xfc,0xff,0xff,0xff};
const byte SUBH_COLNAME[] = {0xff,0xff,0xff,0xff};
const byte SUBH_COLLABS[] = {0xfe,0xfb,0xff,0xff}; 
const byte SUBH_COLLIST[] = {0xfe,0xff,0xff,0xff};
const byte SUBH_SUBHCNT[] = {0x00,0xfc,0xff,0xff};

/* subh ptr info */
const size_t SUBH_NUM_PTR_OFFSET = 20;
const size_t SUBH_PTR_OFFSET = 24;
const size_t SUBH_PTR_LENGTH = 12;
const size_t SUBH_LENGTH_OFFSET = 4;

/* colattr info */
const size_t COLATTR_INFO_OFFSET = 12;
const size_t COLATTR_FILLER = 8;
const size_t COLATTR_INFO_LENGTH = 12;
const size_t COLATTR_COL_WIDTH_OFFSET = 4;
const size_t COLATTR_COL_TYPE_OFFSET = 10;

/* colname info */
const size_t COLNAME_PTR_OFFSET = 12;
const size_t COLNAME_PTR_LENGTH = 8;
const size_t COLNAME_FILLER = 8;
const size_t COLNAME_PTR_NAME_OFFSET = 2;
const size_t COLNAME_PTR_LENGTH_OFFSET = 4;

/* collabs info */
const size_t COLLABS_FORMAT_PTR_OFFSET = 34;
const size_t COLLABS_FORMAT_PTR_OFFSET_OFFSET = 36;
const size_t COLLABS_FORMAT_PTR_LENGTH_OFFSET = 38;
const size_t COLLABS_LAB_PTR_OFFSET = 40;
const size_t COLLABS_LAB_PTR_OFFSET_OFFSET = 42;
const size_t COLLABS_LAB_PTR_LENGTH_OFFSET = 44;

const int SUBH_TYPES_NUM = 8;
enum SUBH_TYPE {
  UNKNOWN=-1,
  ROWSIZE,
  COLSIZE,
  COLTEXT,
  COLATTR,
  COLNAME,
  COLLABS,
  COLLIST,
  SUBHCNT};

const size_t SUBHEADER_PTR_BYTES = 12;

const size_t ROW_LENGTH_OFFSET = 20;
const size_t ROW_COUNT_OFFSET = 24;
const size_t ROW_CC1_OFFSET = 36;
const size_t ROW_CC2_OFFSET = 40;
const size_t ROW_COUNT_MIX_OFFSET = 60;

const size_t COL_COUNT_OFFSET = 4;

/* page constants */
const size_t PAGE_TYPE_OFFSET = 16;
const size_t PAGE_DATA_OFFSET = 24;
const size_t PAGE_DATA_ROW_OFFSET = 18;
const size_t PAGE_MIX_MOD = 8;

enum PAGE_TYPE {
  PAGE_META = 0,
  PAGE_DATA = 256,
  PAGE_MIX = 512,
  PAGE_AMD = 1024};

enum COL_TYPE {
  NUM=1,
  CHAR=2};

/* header structures */
typedef struct {
  size_t size;
  int count;
} page_info;

typedef struct {
  byte *magic_number;
  size_t align;
  char *sas_label;
  char *dataset_name;
  char *file_type;
  double timestamp;
  char *sas_release;
  char *sas_host;
  page_info *page_info_ptr;
} header_info;

/* subheader structures */
typedef struct {  
  int page;
  size_t offset;
  size_t length;
  byte *info;
  enum SUBH_TYPE type;
} subh_ptr;

typedef struct {  
  subh_ptr **ptrs;
  int count;
  int *type_counts;
} subh_array;

typedef struct{
  size_t length;  
  int count;
  int count_mix;  
} row_info;

typedef struct{
  char *name;
  char *label;
  char *format;
  size_t offset;
  size_t length;
  enum COL_TYPE type;
} col;

typedef struct{
  col **cols;
  int count;
} col_info;

typedef struct {
  row_info *r;
  col_info *c;
  enum PAGE_TYPE *page_type;
} meta_info;


/* function templates */
void to_csv(FILE *data_file, FILE *out_file, FILE *info_file);

/* header functions */
byte * get_header(FILE *data_file);
header_info * parse_header(byte *header);
byte * get_magic_number(byte *header);
void check_magic_number(byte *magic_number);
size_t get_alignment(byte *header);
char * get_sas_label(byte *header);
char * get_dataset_name(byte *header);
char * get_file_type(byte *header);
double get_timestamp(byte *header, size_t align);
page_info * get_page_info(byte *header, size_t align);
char * get_sas_release(byte *header, size_t align);
char * get_sas_host(byte *header, size_t align);
void write_header_info(FILE *info_file, header_info *header_info_ptr);

/* subheader functions */
meta_info * get_meta_info(FILE *data_file, page_info *page_info_ptr);
subh_array * initialize_subh_array();
void get_subhs(subh_array *subhs, byte *page, int page_num);
subh_ptr ** get_subh(subh_array *subhs, enum SUBH_TYPE type);
enum SUBH_TYPE get_subh_type(byte *signature);
subh_ptr * initialize_subh_ptr(int ptr_num, byte *page, int page_num);
void add_to_subh_array(subh_array *subhs, subh_ptr *ptr);
row_info * parse_rowsize(subh_ptr **subhs, int n_subhs);
col_info * parse_colheaders(subh_ptr **colsize, int n_colsize,
                            subh_ptr **coltext, int n_coltext,
                            subh_ptr **colattr, int n_colattr,
                            subh_ptr **colname, int n_colname,
                            subh_ptr **collabs, int n_collabs);
void get_colattr(col_info *col_info_ptr,
                 subh_ptr **colattr, int n_colattr);
void get_colname(col_info *col_info_ptr,
                 subh_ptr **colname, int n_colname,
                 subh_ptr **coltext, int n_coltext);
void get_collabs(col_info *col_info_ptr,
                 subh_ptr **collabs, int n_collabs,
                 subh_ptr **coltext, int n_coltext);
void write_meta_info(FILE *info_file, meta_info *ptr);

/* data functions */
void write_header(FILE *out_file, col_info *col_info_ptr);
void write_page(FILE *out_file, byte *page, int page_num, meta_info *meta_info_ptr);
void write_char(FILE *out_file, byte *str, int length);
void write_num(FILE *out_file, byte *num, int length);

/* utility functions */
char * trim(char *str);


int main(int argc, char *argv[]) {  
  if (argc < 2) {
    fprintf(stderr, "%s: no file given\n", argv[0]);
    exit(1);
  }

  FILE *data_file = fopen(argv[1], "rb");
  char *info_file_name = (char *) malloc(sizeof(char)*100);
  char *out_file_name = (char *) malloc(sizeof(char)*100);
  strcpy(info_file_name, argv[1]);
  strcpy(out_file_name, argv[1]);
  FILE *info_file = fopen(strcat(info_file_name,".info"), "wb");
  FILE *out_file = fopen(strcat(out_file_name,".csv"), "wb");
  setvbuf(out_file, NULL, _IOFBF, 1024*512);

  to_csv(data_file, out_file, info_file);

  fclose(data_file);
  fflush(info_file);
  fclose(info_file);
  fflush(out_file);
  fclose(out_file);
  return 0;
}

void to_csv(FILE *data_file, FILE *out_file, FILE *info_file) {
  byte *header = get_header(data_file);
  header_info *header_info_ptr = parse_header(header);
  write_header_info(info_file, header_info_ptr);
    
  meta_info *meta_info_ptr = get_meta_info(data_file, header_info_ptr->page_info_ptr);
  write_meta_info(info_file, meta_info_ptr);

  write_header(out_file, meta_info_ptr->c);

  int i;
  byte *page = (byte *) malloc(sizeof(byte)*header_info_ptr->page_info_ptr->size);
  for (i = 0; i < header_info_ptr->page_info_ptr->count; i++) {
    fread(page, sizeof(byte), header_info_ptr->page_info_ptr->size, data_file);
    write_page(out_file, page, i, meta_info_ptr);
  }
  free(page);
}

/* header functions */
byte * get_header(FILE *data_file) {
  byte *header = (byte *) malloc(sizeof(byte)*HEADER_LENGTH);
  fread(header, sizeof(byte), HEADER_LENGTH, data_file);
  return header;
}

header_info * parse_header(byte *header) {
  header_info *header_info_ptr = (header_info *) malloc(sizeof(header_info));
  header_info_ptr->magic_number = get_magic_number(header);
  header_info_ptr->align = get_alignment(header);
  header_info_ptr->sas_label = get_sas_label(header);
  header_info_ptr->dataset_name = get_dataset_name(header);
  header_info_ptr->file_type = get_file_type(header);
  header_info_ptr->timestamp = get_timestamp(header,header_info_ptr->align);
  header_info_ptr->page_info_ptr = get_page_info(header,header_info_ptr->align);
  header_info_ptr->sas_release = get_sas_release(header,header_info_ptr->align);
  header_info_ptr->sas_host = get_sas_host(header,header_info_ptr->align);       
  return header_info_ptr;
}

byte * get_magic_number(byte *header) {
  byte *magic_number = (byte *) malloc(sizeof(byte)*MAGIC_NUMBER_LENGTH);
  memcpy(magic_number, header, MAGIC_NUMBER_LENGTH);
  check_magic_number(magic_number);
  return magic_number;
}

void check_magic_number(byte *magic_number) {
  if (memcmp(magic_number, REAL_MAGIC_NUMBER, MAGIC_NUMBER_LENGTH) != 0) {
    fprintf(stderr,"error: magic number does not match\n");
    exit(1);
  }
}

size_t get_alignment(byte *header) {
  byte *align = (byte *) malloc(ALIGN_LENGTH*sizeof(byte));
  memcpy(align, header+ALIGN_OFFSET, ALIGN_LENGTH);
  if (memcmp(align, ALIGN_64, ALIGN_LENGTH) == 0) return ALIGN_64_OFFSET;
  if (memcmp(align, ALIGN_32, ALIGN_LENGTH) == 0) return ALIGN_32_OFFSET;
  fprintf(stderr,"error: unrecognized alignment code\n");
  exit(2);  
}

char * get_sas_label(byte *header) {
  byte *sas_label = (byte *) malloc(sizeof(byte)*SAS_LABEL_LENGTH);
  memcpy(sas_label, header+SAS_LABEL_OFFSET, SAS_LABEL_LENGTH);
  return trim(sas_label);
}

char * get_dataset_name(byte *header) {
  byte *dataset_name = (byte *) malloc(sizeof(byte)*DATASET_NAME_LENGTH);
  memcpy(dataset_name, header+DATASET_NAME_OFFSET, DATASET_NAME_LENGTH);
  return trim(dataset_name);
}

char * get_file_type(byte *header) {
  byte *file_type = (byte *) malloc(sizeof(byte)*FILE_TYPE_LENGTH);
  memcpy(file_type, header+FILE_TYPE_OFFSET, FILE_TYPE_LENGTH);
  return trim(file_type);
}

double get_timestamp(byte *header, size_t align) {
  double timestamp;
  memcpy(&timestamp, header+TIMESTAMP_OFFSET+align, sizeof(double));
  return timestamp;
}

page_info * get_page_info(byte *header, size_t align) {
  page_info *page_info_ptr = (page_info *) malloc(sizeof(page_info));
  int page_size;
  memcpy(&page_size, header+PAGE_SIZE_OFFSET+align, sizeof(int));
  if (page_size < 0) fprintf(stderr,"error page size is negative\n");
  page_info_ptr->size = page_size;

  memcpy(&page_info_ptr->count, header+PAGE_COUNT_OFFSET+align, sizeof(int));
  if (page_info_ptr->count < 1) fprintf(stderr,"error page count is not positive\n");
    
  return page_info_ptr;
}

char * get_sas_release(byte *header, size_t align) {
  byte *sas_release = (byte*) malloc(sizeof(byte)*SAS_RELEASE_LENGTH);
  memcpy(sas_release, header+SAS_RELEASE_OFFSET+align, SAS_RELEASE_LENGTH);
  return trim(sas_release);
}

char * get_sas_host(byte *header, size_t align) {
  byte *sas_host = (byte*) malloc(sizeof(byte)*SAS_HOST_LENGTH);
  memcpy(sas_host, header+SAS_HOST_OFFSET+align, SAS_HOST_LENGTH);
  return trim(sas_host);
}

void write_header_info(FILE *info_file, header_info *header_info_ptr) {  
  fprintf(info_file, "Alignment: %zd\n",header_info_ptr->align);  
  fprintf(info_file, "SAS label: %s\n",header_info_ptr->sas_label); /* expect SAS FILE */  
  fprintf(info_file, "Dataset name: %s\n",header_info_ptr->dataset_name);
  fprintf(info_file, "File type: %s\n",header_info_ptr->file_type);
  fprintf(info_file, "Timestamp: %lf seconds since 1960-01-01\n",header_info_ptr->timestamp);
  fprintf(info_file, "Page size: %zd bytes\n",header_info_ptr->page_info_ptr->size);
  fprintf(info_file, "Page count: %d\n",header_info_ptr->page_info_ptr->count);
  fprintf(info_file, "SAS release: %s\n",header_info_ptr->sas_release);
  fprintf(info_file, "SAS host: %s\n",header_info_ptr->sas_host);
}

/* end of header functions */

meta_info * get_meta_info(FILE *data_file, page_info *page_info_ptr) {
  meta_info *meta_info_ptr = (meta_info *) malloc(sizeof(meta_info));

  /* read in subheaders and page types */
  subh_array *subhs = initialize_subh_array();
  meta_info_ptr->page_type = (enum PAGE_TYPE *) malloc(sizeof(enum PAGE_TYPE)
                                                       *page_info_ptr->count);
  int i;
  for (i = 0; i < page_info_ptr->count; i++) {
    fseek(data_file, PAGE_TYPE_OFFSET, SEEK_CUR);
    short p_type;
    fread(&p_type, sizeof(short), 1, data_file);
    meta_info_ptr->page_type[i] = (enum PAGE_TYPE) p_type;
    if (meta_info_ptr->page_type[i] != PAGE_DATA) {
      fseek(data_file, -PAGE_TYPE_OFFSET-sizeof(short), SEEK_CUR);
      byte *page = (byte *) malloc(sizeof(byte)*page_info_ptr->size);
      fread(page, sizeof(byte), page_info_ptr->size, data_file);
      get_subhs(subhs, page, i);
      free(page);
    } else {
      fseek(data_file, page_info_ptr->size-PAGE_TYPE_OFFSET-sizeof(short), SEEK_CUR);
    }    
  }

  fseek(data_file, HEADER_LENGTH, SEEK_SET); /* reset to header */

  meta_info_ptr->r = parse_rowsize(get_subh(subhs, ROWSIZE),
                                   subhs->type_counts[ROWSIZE]);

  meta_info_ptr->c = parse_colheaders(get_subh(subhs, COLSIZE),subhs->type_counts[COLSIZE],
                                      get_subh(subhs, COLTEXT),subhs->type_counts[COLTEXT],
                                      get_subh(subhs, COLATTR),subhs->type_counts[COLATTR],
                                      get_subh(subhs, COLNAME),subhs->type_counts[COLNAME],
                                      get_subh(subhs, COLLABS),subhs->type_counts[COLLABS]);

  /* free subhs */
  for (i = 0; i < subhs->count; i++) free(subhs->ptrs[i]->info);
  free(subhs->ptrs);
  free(subhs->type_counts);
  free(subhs);

  return meta_info_ptr;
}

subh_array * initialize_subh_array() {
  subh_array *subhs = (subh_array *) malloc(sizeof(subh_array));
  subhs->count = 0;
  subhs->type_counts = (int *) calloc(SUBH_TYPES_NUM, sizeof(int));
  int n_ptrs = 200;
  subhs->ptrs = (subh_ptr **) malloc(sizeof(subh_ptr *)*n_ptrs);
  int i;
  for (i = 0; i < n_ptrs; i++) subhs->ptrs[i] = (subh_ptr *) malloc(sizeof(subh_ptr));
  return subhs;
}

void get_subhs(subh_array *subhs, byte *page, int page_num) {
  short n_subhs;
  memcpy(&n_subhs, page+SUBH_NUM_PTR_OFFSET, sizeof(short));

  subh_ptr *ptr;  
  int i;
  for (i = 0; i < n_subhs; i++) add_to_subh_array(subhs,
                                                  initialize_subh_ptr(i,
                                                                      page,
                                                                      page_num));
}

subh_ptr ** get_subh(subh_array *subhs, enum SUBH_TYPE type) {
  subh_ptr **ptrs = (subh_ptr **) malloc(sizeof(subh_ptr *)*subhs->type_counts[type]);
  int i;
  for (i = 0; i < subhs->type_counts[type]; i++) {
    ptrs[i] = (subh_ptr *) malloc(sizeof(subh_ptr)*subhs->type_counts[type]);
  }

  int current = 0;
  i = 0;
  for (i = 0; i < subhs->count; i++) {
    if (subhs->ptrs[i]->type == type) {
      ptrs[current] = subhs->ptrs[i];
      current += 1;
      if (current == subhs->type_counts[type]) break;
    }
  }
  
  return ptrs;
}

byte * get_subh_signature(subh_ptr *ptr, byte *page) {
  byte *signature = (byte *) malloc(sizeof(byte)*SUBH_SIG_LENGTH);
  memcpy(signature, page+ptr->offset, SUBH_SIG_LENGTH);
  return signature;
}

enum SUBH_TYPE get_subh_type(byte *signature) {
  if ( memcmp(signature, SUBH_ROWSIZE, SUBH_SIG_LENGTH ) == 0 ) return ROWSIZE;
  if ( memcmp(signature, SUBH_COLSIZE, SUBH_SIG_LENGTH ) == 0 ) return COLSIZE;
  if ( memcmp(signature, SUBH_COLTEXT, SUBH_SIG_LENGTH ) == 0 ) return COLTEXT;
  if ( memcmp(signature, SUBH_COLATTR, SUBH_SIG_LENGTH ) == 0 ) return COLATTR;
  if ( memcmp(signature, SUBH_COLNAME, SUBH_SIG_LENGTH ) == 0 ) return COLNAME;
  if ( memcmp(signature, SUBH_COLLABS, SUBH_SIG_LENGTH ) == 0 ) return COLLABS;
  if ( memcmp(signature, SUBH_COLLIST, SUBH_SIG_LENGTH ) == 0 ) return COLLIST;
  if ( memcmp(signature, SUBH_SUBHCNT, SUBH_SIG_LENGTH ) == 0 ) return SUBHCNT;
  return UNKNOWN;
}

subh_ptr * initialize_subh_ptr(int ptr_num, byte *page, int page_num) {
  byte *raw_ptr = (byte *) malloc(sizeof(byte)*SUBH_PTR_LENGTH);
  memcpy(raw_ptr, page+SUBH_PTR_OFFSET+ptr_num*SUBH_PTR_LENGTH,SUBH_PTR_LENGTH);
  subh_ptr *ptr = (subh_ptr *) malloc(sizeof(subh_ptr));
  int offset;
  int length;
  memcpy(&offset, raw_ptr, sizeof(int));
  memcpy(&length, raw_ptr+SUBH_LENGTH_OFFSET, sizeof(int));
  ptr->offset = offset;
  ptr->length = length;
  byte *sig = get_subh_signature(ptr, page);
  ptr->type = get_subh_type(sig);
  ptr->page = page_num;
  ptr->info = (byte *) malloc(sizeof(byte *)*ptr->length);
  memcpy(ptr->info, page+ptr->offset, ptr->length);
  return ptr;
}

void add_to_subh_array(subh_array *subhs, subh_ptr *ptr) {
  if (ptr->type == UNKNOWN) return;
  subhs->ptrs[subhs->count] = ptr;
  subhs->count++;
  subhs->type_counts[ptr->type]++;
}

row_info * parse_rowsize(subh_ptr **subhs, int n_subhs) {
  row_info *ptr = (row_info *) malloc(sizeof(row_info));
  int i;
  for (i = 0; i < n_subhs; i++) {
    memcpy(&ptr->length, subhs[i]->info + ROW_LENGTH_OFFSET, sizeof(int));
    memcpy(&ptr->count, subhs[i]->info + ROW_COUNT_OFFSET, sizeof(int));
    memcpy(&ptr->count_mix, subhs[i]->info + ROW_COUNT_MIX_OFFSET, sizeof(int));
  }

  return ptr;
}

col_info * parse_colheaders(subh_ptr **colsize, int n_colsize,
                            subh_ptr **coltext, int n_coltext,
                            subh_ptr **colattr, int n_colattr,
                            subh_ptr **colname, int n_colname,
                            subh_ptr **collabs, int n_collabs) {
  col_info *ptr = (col_info *) malloc(sizeof(col_info));

  /* first get column count */
  int i;
  for (i=0; i < n_colsize; i++) {
    memcpy(&ptr->count, colsize[i]->info+COL_COUNT_OFFSET, sizeof(int));
  }

  /* allocate memory for columns */
  ptr->cols = (col **) malloc(sizeof(col *)*ptr->count);
  for (i=0; i < ptr->count; i++) ptr->cols[i] = (col *) malloc(sizeof(col));

  /* get colattr */
  get_colattr(ptr,
              colattr, n_colattr);
  
  /* get colname */
  get_colname(ptr,
              colname, n_colname,
              coltext, n_coltext);

  /* get collabs */
  get_collabs(ptr,
              collabs, n_collabs,
              coltext, n_coltext);
  
  return ptr;
}

void get_colattr(col_info *col_info_ptr,
                 subh_ptr **colattr, int n_colattr) {
  int i, j, n_attr, base, offset, length;
  byte type;
  int current = 0;
  for (i = 0; i < n_colattr; i++) {
    n_attr = (colattr[i]->length-COLATTR_INFO_OFFSET-COLATTR_FILLER)/COLATTR_INFO_LENGTH;
    for (j = 0; j < n_attr; j++) {
      base = COLATTR_INFO_OFFSET+j*COLATTR_INFO_LENGTH;
      memcpy(&offset,
             colattr[i]->info+base,
             sizeof(int));
      memcpy(&length,
             colattr[i]->info+base+COLATTR_COL_WIDTH_OFFSET,
             sizeof(int));
      memcpy(&type,
             colattr[i]->info+base+COLATTR_COL_TYPE_OFFSET,
             sizeof(byte));
      if (type != NUM && type != CHAR) {
        fprintf(stderr,"error: unrecognized data type in column %d\n", current);
        exit(1);
      }
      col_info_ptr->cols[current]->offset = offset;
      col_info_ptr->cols[current]->length = length;
      col_info_ptr->cols[current]->type = (enum COL_TYPE) type;
      current++;
    }
  }
}

void get_colname(col_info *col_info_ptr,
                 subh_ptr **colname, int n_colname,
                 subh_ptr **coltext, int n_coltext) {
  int i, j, n_ptr, base;
  short idx, offset, length;
  byte *name;

  int current = 0;
  for (i = 0; i < n_colname; i++) {
    n_ptr = (colname[i]->length-COLNAME_PTR_OFFSET-COLNAME_FILLER)/COLNAME_PTR_LENGTH;
    for (j = 0; j < n_ptr; j++ ) {
      base = COLNAME_PTR_OFFSET+j*COLNAME_PTR_LENGTH;            
      memcpy(&idx, colname[i]->info + base, sizeof(short));
      memcpy(&offset, colname[i]->info + base + COLNAME_PTR_NAME_OFFSET, sizeof(short));
      memcpy(&length, colname[i]->info + base + COLNAME_PTR_LENGTH_OFFSET, sizeof(short));
      
      name = (byte *) malloc(sizeof(byte)*length);      
      memcpy(name,
             coltext[idx]->info + offset + SUBH_SIG_LENGTH,
             length);
      col_info_ptr->cols[current]->name = name;
      current++;
    }
  }
}

void get_collabs(col_info *col_info_ptr,
                 subh_ptr **collabs, int n_collabs,
                 subh_ptr **coltext, int n_coltext) {

  int i;
  short format_idx, format_offset, format_length, lab_idx, lab_offset, lab_length;
  byte *format;
  byte *lab;
  for (i = 0; i < n_collabs; i++) {
    if (collabs[i]->length == 0) continue;
    memcpy(&format_idx,
           collabs[i]->info + COLLABS_FORMAT_PTR_OFFSET,
           sizeof(short));
    memcpy(&format_offset,
           collabs[i]->info + COLLABS_FORMAT_PTR_OFFSET_OFFSET,
           sizeof(short));
    memcpy(&format_length,
           collabs[i]->info + COLLABS_FORMAT_PTR_LENGTH_OFFSET,
           sizeof(short));
    memcpy(&lab_idx,
           collabs[i]->info + COLLABS_LAB_PTR_OFFSET,
           sizeof(short));
    memcpy(&lab_offset,
           collabs[i]->info + COLLABS_LAB_PTR_OFFSET_OFFSET,
           sizeof(short));
    memcpy(&lab_length,
           collabs[i]->info + COLLABS_LAB_PTR_LENGTH_OFFSET,
           sizeof(short));
    
    if (format_length > 0) {
      format = (byte *) malloc(sizeof(byte)*format_length);
      memcpy(format,
             coltext[format_idx]->info+format_offset+SUBH_SIG_LENGTH,
             format_length);
      col_info_ptr->cols[i]->format=format;
    } else {
      col_info_ptr->cols[i]->format="NULL";
    }
    
    if (lab_length > 0) {
      lab = (byte *) malloc(sizeof(byte)*lab_length);
      memcpy(lab,
             coltext[lab_idx]->info+lab_offset+SUBH_SIG_LENGTH,
             lab_length);    
      col_info_ptr->cols[i]->label=lab;
    } else {
      col_info_ptr->cols[i]->label="NULL";
    }
  }  
}

void write_meta_info(FILE *info_file, meta_info *ptr) {
  fprintf(info_file, "Row length: %zd bytes\n", ptr->r->length);
  fprintf(info_file, "Row count: %d\n", ptr->r->count);
  fprintf(info_file, "Row count mix: %d\n", ptr->r->count_mix);

  /* write column info */
  fprintf(info_file, "\nColumn Information\n\n");
  fprintf(info_file, "Column count: %d\n", ptr->c->count);
  int i;
  
  for (i = 0; i < ptr->c->count; i++) {
    fprintf(info_file, "Column number: %d\n", i);
    fprintf(info_file, "Name: %s\n", ptr->c->cols[i]->name);
    fprintf(info_file, "Offset: %zd bytes\n", ptr->c->cols[i]->offset);
    fprintf(info_file, "Length: %zd bytes\n", ptr->c->cols[i]->length);
    if (ptr->c->cols[i]->type == NUM) {
      fprintf(info_file, "Type: %s\n", "numeric");
    } else {
      fprintf(info_file, "Type: %s\n", "character");
    }
    fprintf(info_file,"Format: %s\n",ptr->c->cols[i]->format);
    fprintf(info_file,"Label: %s\n\n",ptr->c->cols[i]->label);
  }
}

void write_header(FILE *out_file, col_info *col_info_ptr) {
  int i;
  for (i = 0; i < col_info_ptr-> count; i++) {
    fprintf(out_file, "\"%s\"",col_info_ptr->cols[i]->name);
    if (i != col_info_ptr-> count - 1) fprintf(out_file,",");
  }
  fprintf(out_file, "\n");
}

void write_page(FILE *out_file, byte *page, int page_num, meta_info *meta_info_ptr) {
  if (meta_info_ptr-> page_type[page_num] == PAGE_META ||
      meta_info_ptr-> page_type[page_num] == PAGE_AMD) return;

  int r, c, row_count;
  size_t data_offset, col_offset, col_length;
  
  if (meta_info_ptr-> page_type[page_num] == PAGE_MIX) {
    short n_subhs;
    memcpy(&n_subhs, page + SUBH_NUM_PTR_OFFSET, sizeof(short));
    data_offset = PAGE_DATA_OFFSET + n_subhs*SUBH_PTR_LENGTH;
    data_offset = data_offset + data_offset % PAGE_MIX_MOD;
    row_count = meta_info_ptr->r->count_mix;    
  } else {                      /* page is a data page */
    data_offset = PAGE_DATA_OFFSET;
    memcpy(&row_count, page+PAGE_DATA_ROW_OFFSET, sizeof(int));
  }

  /* if there are too many rows in the mix row count it's not right */
  if (row_count > meta_info_ptr->r->count) row_count = meta_info_ptr->r->count;

  for (r = 0; r < row_count; r++) {
    for (c = 0; c < meta_info_ptr->c->count; c++) {
      col_offset = meta_info_ptr->c->cols[c]-> offset;
      col_length = meta_info_ptr->c->cols[c]->length;
      if (meta_info_ptr->c->cols[c]->type == NUM) {
        write_num(out_file, page + data_offset + col_offset, col_length);
      } else {
        write_char(out_file, page + data_offset + col_offset, col_length);
      }
      if (c != meta_info_ptr->c->count - 1) fprintf(out_file,",");
    }
    fprintf(out_file,"\n");
    data_offset += meta_info_ptr->r->length;
  }
}

void write_char(FILE *out_file, byte *str, int length) {
  byte *byte_str = (byte *) malloc(sizeof(byte)*length+1);
  memcpy(byte_str, str, length);
  byte_str[length] = '\0';
  char *char_str = byte_str;
  char_str = trim(char_str);

  fprintf(out_file,"\"%s\"",char_str);
  
  free(char_str);
}

void write_num(FILE *out_file, byte *num, int length) {
  double new_num;
  memset(&new_num,0,sizeof(double)-length);
  memcpy(&new_num + (sizeof(double) - length) , num, length);
  fprintf(out_file,"%lf",new_num);         
}

/* trim leading and trailing whitespace in place*/
char * trim(char *str) {
  int start = 0;
  int end = strlen(str);
  while ( start < end && isspace(str[start]) ) start++;
  while ( end > start && isspace(str[end-1]) ) end--;
  memmove(str,str + start,end-start);
  str[end-start] = '\0';
  return str;
}
