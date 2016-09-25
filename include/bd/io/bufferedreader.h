#ifndef bd_buffered_reader
#define bd_buffered_reader

#include "readerworker.h"
#include "bufferpool.h"
#include "buffer.h"

#include <fstream>
#include <thread>
#include <mutex>
#include <future>

namespace bd
{

template<typename Ty>
class BufferedReader
{

public:
  BufferedReader(size_t bufSize);


  ~BufferedReader();


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Open the raw file at path.
  /// \return True if opened, false otherwise.
  ///////////////////////////////////////////////////////////////////////////////
  bool
  open(std::string const &path);


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Start readering the file.
  ///////////////////////////////////////////////////////////////////////////////
  void
  start();


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Stop reading as soon as possible.
  ///////////////////////////////////////////////////////////////////////////////
//  void stop();


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Stop reading and wait for the read thread to exit.
  /// \return The number of bytes read so far by the thread.
  ///////////////////////////////////////////////////////////////////////////////
  long long int
  reset();


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Return true if the read thread is still active.
  ///////////////////////////////////////////////////////////////////////////////
  bool
  isReading() const;


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Return true if the buffer pool has full buffers and/or the reader
  ///        is still reading from the file.
  ///////////////////////////////////////////////////////////////////////////////
  bool
  hasNext() const;


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Grab the next buffer data from the pool as soon as it is ready.
  /// \note Blocks until a full buffer is ready.
  ///////////////////////////////////////////////////////////////////////////////
  Buffer<Ty> *
  waitNext()
  {
    return m_pool->nextFull();
  }


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Return a buffer to the empty pool to be filled again.
  ///////////////////////////////////////////////////////////////////////////////
  void
  waitReturn(Buffer<Ty> *buf)
  {
    return m_pool->returnEmpty(buf);
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// /brief Get the number of elements in a single buffer in the buffer pool.
  ///////////////////////////////////////////////////////////////////////////////
  size_t
  singleBufferElements() {
    return m_pool->bufferSizeElements();
  }

  size_t totalBufferBytes() {
    return m_bufSizeBytes;
  }

private:
  std::string m_path;

  size_t m_bufSizeBytes;
  int m_numBuffers;

  BufferPool<Ty> *m_pool;
  std::future<long long int> m_future;
  std::atomic_bool m_stopThread;

};


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
BufferedReader<Ty>::BufferedReader(size_t bufSize)
    : m_path{ }
    , m_bufSizeBytes{ bufSize }
    , m_numBuffers{ 4 }
    , m_pool{ nullptr }
    , m_future{ }
{
}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
BufferedReader<Ty>::~BufferedReader()
{
  if (m_pool) {
    delete m_pool;
  }
}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
bool
BufferedReader<Ty>::open(std::string const &path)
{
  m_path = path;
  std::ifstream test(m_path);
  if (! test.is_open()) {
    Err() << "Unable to open file: " + m_path;
    return false;
  }
  test.close();
  m_pool = new BufferPool<Ty>(m_bufSizeBytes, 4);
  m_pool->allocate();
  return true;

}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
void
BufferedReader<Ty>::start()
{
  m_stopThread = false;
  m_future =
      std::async(std::launch::async,
                 [&]() -> long long int {
                   ReaderWorker<BufferedReader<Ty>, BufferPool<Ty>, Ty>
                       worker(this, m_pool);
                   worker.setPath(m_path);
                   return worker(std::ref(m_stopThread));
                 });
}


///////////////////////////////////////////////////////////////////////////////
//template<typename Ty>
//void
//BufferedReader<Ty>::stop()
//{
//  m_stopThread = true;
//}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
long long int
BufferedReader<Ty>::reset()
{
  m_stopThread = true;
  m_future.wait();
  m_pool->reset();
  return m_future.get();
}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
bool
BufferedReader<Ty>::isReading() const
{
  return m_future.wait_for(std::chrono::seconds(0)) !=
      std::future_status::ready;
}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
bool
BufferedReader<Ty>::hasNext() const
{
  // if no buffers in pool, check if the reader thread is done.
  // If it is still reading, then we should return true because there will
  // be a full buffer available soon.
  return m_pool->hasNext() || isReading();
}


} // namespace preproc
#endif // ! buffered_reader__
