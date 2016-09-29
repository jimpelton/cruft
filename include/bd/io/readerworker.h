#ifndef readerworker_h__
#define readerworker_h__

#include <bd/log/logger.h>
#include <bd/io/bufferpool.h>
#include <bd/io/bufferedreader.h>
#include <bd/io/buffer.h>


#include <fstream>
#include <atomic>
#include <string>

namespace bd
{


template<class Reader, class Pool, class Ty>
class ReaderWorker
{
public:
  ReaderWorker(Reader *r, Pool *p)
    : m_reader{ r }
    , m_pool{ p }
    , m_is{ nullptr }
  { }

  ~ReaderWorker()
  {
    if (m_is)
      delete m_is;
  }

  /// \brief Pop buffers and fill them from the ifstream.
  /// \returns -1 if file could not be opened, or the total bytes read.
  long long
  operator()(std::atomic_bool const &quit)
  {
    if (! open()) {
        Err() << "Could not open file " << m_path << ". Exiting readerworker loop.";
        return -1;
    }

    // bytes to attempt to read from file.
    size_t const buffer_size_bytes{ m_pool->bufferSizeElements() * sizeof(Ty) };
    size_t total_read_bytes{ 0 };

    Info() << "Starting reader loop.";
    while(!m_is->eof() && !quit) {
//      Dbg() << "Reader waiting for empty buffer.";

      // wait for the next empty buffer in the pool.
      Buffer<Ty> *buf = m_pool->nextEmpty();
      // set the element index this buffer starts at.
      buf->setIndexOffset(total_read_bytes/sizeof(Ty));
      Ty *data = buf->getPtr();
        
//      Dbg() << "Reader filling buffer.";
      m_is->read(reinterpret_cast<char*>(data), buffer_size_bytes);
      std::streampos amount{ m_is->gcount() };
//      Dbg() << "Reader filled buffer with " << amount << " bytes.";
      
      // the last buffer filled may not be a full buffer, so resize!
      if ( amount < static_cast<long long>(buffer_size_bytes) ) {

        buf->setNumElements(amount/sizeof(Ty));
        if (amount == 0) {
          Dbg() << "Reader read 0 bytes.";
          m_pool->returnEmpty(buf);
//          Dbg() << "Reader done returning empty buffer.";
          break;
        }
      } // if (amount...)
        
//      Dbg() << "Reader returning full buffer.";
      m_pool->returnFull(buf);
//      Dbg() << "Reader done returning full buffer.";

      total_read_bytes += amount;
      Info() << "Read " << total_read_bytes << " bytes.";
    } // while

    m_is->close();
    Info() << "Reader done after reading " << total_read_bytes << " bytes";
    m_pool->kickThreads();
    return total_read_bytes;
  }

    
  void
  setPath(std::string const &path)
  {
    m_path = path;
  }


private:
  bool
  open()
  {
    m_is = new std::ifstream();
    m_is->open(m_path, std::ios::binary);
    return m_is->is_open();
  }

  Reader *m_reader;
  Pool *m_pool;
  std::ifstream *m_is;
  std::string m_path;

}; // ReaderWorker


} //namespace preproc

#endif // ! readerworker_h__
