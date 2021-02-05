/**
 Project 3: File copying
 Author: Hanshi Zuo
 Program to copy files. Enter source followed by destination
 files and then add flags as needed. New source can be created
 automatically.Flags -t for truncating, -a for appending, and -b
 to specify the number of bytes transfered at a time (default 64).
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cctype>
using std::printf;
using std::strncmp;
using std::malloc;
using std::free;

/// File  base class
class File
{
   public:
   File();

   /* setter for path
      :path: the inputted path
   */
   void setname(char * path)
   {
       name = path;
   }

   virtual void Open() {}

   /* check to see if file properly opened
      :return: 1 for openable and usable
   */
   bool accessable()
   {
        return id >= 0;
   }
   virtual ~File();

   protected:
   char * name = nullptr;
   int id = -1;
};

/// derived class for reading
class RFile : public File
{
    public:
    RFile();
    void Open();
    ssize_t Read(void * , size_t);
};

/// derived class for writing
class WFile : public File
{
    public:
    WFile();
    void Open();

    ssize_t Write(const void *, size_t);
    /* setter for the write flags
       :f: the new flag value
    */
    void setflags(int f)
    {
       if(flags == -1)
          flags = f;
       else
          printf("Flag previously entered, will use first one.\n");
    }
    ~WFile();
    private:
    int flags = -1;
    int wcalls = 0;
};

bool parse_flags(char * flag, char * next, WFile * fp, size_t * sz);
void printusage();

/// granting user rwx permissions and everyone else r permission
const mode_t general_read = S_IRWXU | S_IROTH | S_IRGRP;

/* main function to recieve command line
   arguments
   :argc: the number of arguments
   :args: the argument strings
   :return: 0 for succesful completion
*/
int main(int argc, char * args[])
{
    printf("%d arguments entered!\n", argc);
    if (argc < 2)
    {
        printusage();
        return 0;
    }
    if (argc == 2)
    {
        printf("Not enough files provided!\n");
        return 0;
    }
    size_t buf_sz = 64;
    RFile source;
    WFile dest;

    /// loop to collect our command line arguments
    for(int i = 1, files = 0; i < argc ; ++i)
    {
        if( **(args+i) == '-')
           parse_flags(*(args+i), *(args+i+1), &dest, &buf_sz) ? ++i : i;
        else
        {
            ++files;
            if (files  == 1)
                source.setname(*(args+i));
            else if (files == 2)
                dest.setname(*(args+i));
            else
                printf("!Warning! %d files inputted, will use first two.\n",
                files);
        }
    }

    /// opening up files
    source.Open();
    dest.Open();

    /// allocating memory for buffer
    void * temp = malloc(buf_sz);
    size_t transfer = buf_sz;

    /// if both files succesfully opened we can transfer data
    /// we transfer as long as read reads more than zero bytes
    /// loop terminates when this occurs to minimize number of writes
    if(dest.accessable() && source.accessable())
        while((transfer = source.Read(temp, buf_sz)) > 0)
           dest.Write(temp, transfer);

    else if(!source.accessable())
        printf("Cannot read/open input file.\n");
    else
        printf("Cannot write/open second provided file.\n");

    free(temp);

    return 0;
}

/*
 Constructor for file class
*/
File::File()
{
   printf("File class created ");
}

/*
 Destructor guarentees that a file will be closed if needed
*/
File::~File()
{
   if(name && accessable())
   {
      printf("Closing %s\n",name);
      close(id);
   }
   else
      printf("Destructor called.\n");
}

/*
 Constructor for reading files
*/
RFile::RFile() : File()
{
    printf("for reading.\n");
}

/*
 Opener with read only setting
*/
void RFile::Open()
{
    id = open(name, O_RDONLY, general_read);
}

/* reading funtion wrapper
   :buf: the buffer we read into
   :sz: the size of the buufer
   :return: the number of bytes read
*/
ssize_t RFile::Read(void * buf, size_t sz)
{
    return read(id, buf, sz);
}

/*
  Constructor for write file
*/
WFile::WFile() : File()
{
    printf("for writing.\n");
}

/*
  File opener for write file
*/
void WFile::Open()
{
    /* If no flag entered, we only create a file to write if no
       file of name already exists
       if flags are set, then we open the file with said flag
    */
    id = (flags != -1) ?
    open(name, O_WRONLY | O_CREAT | flags , general_read):
    open(name, O_WRONLY | O_CREAT | O_EXCL, general_read);
}

/* Write file
   :buf: the buffer we write from
   :sz: the size of the write
   :return: number of bits written
*/
ssize_t WFile::Write(const void * buf, size_t sz)
{
   ++wcalls;
   return write(id, buf, sz);
}

/*
 Destructor for write file reports number of writes
*/
WFile::~WFile()
{
   printf("Total %d writes.\n",wcalls);
}

/* Function to parse flags
   :flag: the flag we input
   :next: the next command line argument
   :return: 1 for need to advance command line agrument for reading a -b value
*/
bool parse_flags(char * flag, char * next, WFile * fp, size_t * sz)
{
   const short NUM_FLAGS = 3;
   const char * valid_fgs[] = {"-a","-b","-t"};
   for(int j = 0; j < NUM_FLAGS ;++j)
   {
      /// invalid flag
      if(j == 2 && strncmp(*(valid_fgs+j),flag,3*sizeof(char)))
         printf("!Warning! Invalid flag entered and ignored: %s\n", flag);

      /// valid flags
      if(!strncmp(*(valid_fgs+j),flag,3*sizeof(char)))
      {
          if(j == 1)
          {
              int new_sz = atoi(next);
              bool numeric = 1;
              /// making sure that inputted value is a number
              for(char * iter = next; *iter ; ++ iter)
                  if (!isdigit(*iter))
                      numeric = 0;
              if(new_sz > 0 && new_sz < 1000000000 && numeric)
              {
                  *sz = new_sz;
                  printf("New buffer size set = %d\n",new_sz);
                  return 1;
              }
              else
                  printf("!Warning! Invalid -b flag value: %s\n",next);
          }
          else if(j == 2)
              fp -> setflags(O_TRUNC);
          else
              fp -> setflags(O_APPEND);
          return 0;
      }
   }
   return 0;
}

/*
 function to print usage
*/
void printusage()
{
     const char usage[] =
    "!!! Usage\nProgram to copy files. Enter source followed by destination \
files and then add flags as needed. New destination can be created \
automatically.\nFlags -t for truncating, -a for appending, and -b \
to specify the number of bytes transfered at a time (default 64). \
If no flag is entered then existing file will not be modified.\n!!!\n";
    printf("%s",usage);
}
