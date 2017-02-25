//
// Created by jim on 2/18/17.
//


#ifndef bd_reader_h
#define bd_reader_h

#include <bd/io/buffer.h>
#include <bd/datastructure/blockingqueue.h>
#include <bd/log/logger.h>
#include <bd/util/util.h>

#include <fstream>
#include <future>

namespace bd
{

template<class Ty>
class ReaderStrategyBase
{
public:
  ReaderStrategyBase()
      //: ReaderStrategyBase{ nullptr }
  {
  }

  ReaderStrategyBase()
      : m_bytes{ 0 }
//      , _cpos{ 0}
  {
  }

  virtual ~ReaderStrategyBase()
  {
  }



protected:
  std::istream *
  in() { return m_in; }

private:
  /// bytes read so far.
  uint64_t m_bytes;
  /// current offset in the file.
//  uint64_t _cpos;

  std::istream *m_in;

};


template<class Ty>
class LinearReader: public ReaderStrategyBase<Ty>
{

public:
  LinearReader(std::istream *in)
    : ReaderStrategyBase{ in }
  { }

  virtual ~LinearReader()
  { }

  long
  read(char *buf, size_t seek_to, long amount)
  {
    in()->read(buf, amount);
    long actual{ in()->gcount() };
  }

private:

};


template<class Ty>
class BlockReader : public ReaderStrategyBase<Ty>
{
public:

  BlockReader()
      : ReaderStrategyBase{ nullptr }
  { }


  BlockReader(std::istream *in, std::shared_ptr<bd::IndexFile> idx)
      : ReaderStrategyBase(in)
  {
  }


  virtual ~BlockReader()
  {

  }


  long
  read(char *buf, size_t blockIndex)
  {
    bd::FileBlock const &b = (*m_idx)[blockIndex];
    fillBlockData(b, in(), m_vx, m_vy);
  }



private:

  void
  fillBlockData(bd::FileBlock const *b, std::istream *infile,
                size_t vX, size_t vY) const
  {

    // block's dimensions in voxels
    glm::u64vec3 const be{ b->voxel_dims[0], b->voxel_dims[1], b->voxel_dims[2] };

    // start element = block index w/in volume * block size
    glm::u64vec3 const start{ b->ijk() * be };
    // block end element = block voxel start voxelDims + block size
    glm::u64vec3 const end{ start + be };

//    size_t const blockRowLength{ be.x };
//    size_t const sizeType{ to_sizeType(b->texture()->dataType()) };

    // byte offset into file to read from
    size_t offset{ b->data_offset };

    uint64_t const buf_len{ be.x * be.y * be.z };
    //TODO: support for than char*
    char *const disk_buf{ new char[buf_len] };
    char *temp = disk_buf;
    // Loop through rows and slabs of volume reading rows of voxels into memory.
    for (uint64_t slab = start.z; slab < end.z; ++slab) {
      for (uint64_t row = start.y; row < end.y; ++row) {

        // seek to start of row
        infile->seekg(offset);

        // read the bytes of current row
        infile->read(temp, bytes.x);
        temp += bytes.x;

        // offset of next row
        offset = bd::to1D(start.x, row + 1, slab, vX, vY);
        offset *= sizeType;
      }
    }
  }



  std::shared_ptr<bd::IndexFile> m_idx;

  size_t m_vx; ///< volume slab width;
  size_t m_vy; ///< Volume slab height;

};



/// Read a file asynchronously. Buffers are cycled from empty queue to full queue.
///
/// The queues are provided by an external source. Cleanup is expected to be handled
/// externally as well.
///
/// Resets the Buffer index offset and size after each fill.
///
/// \tparam Ty The type in the file.
template<class Ty>
class Reader
{

public:
  using buffer_type = typename bd::Buffer<Ty>;
  using queue_type = typename bd::BlockingQueue<buffer_type *>;


  Reader()
      : Reader{ nullptr, nullptr }
  {
  }


  Reader(bd::BlockingQueue<buffer_type *> *full,
         bd::BlockingQueue<buffer_type *> *empty)
      : m_empty{ empty }
      , m_full{ full }
  {
  }


  virtual ~Reader()
  {
  }


public:

  void
  setFull(bd::BlockingQueue<buffer_type *> *full)
  {
    m_full = full;
  }


  void
  setEmpty(bd::BlockingQueue<buffer_type *> *empty)
  {
    m_empty = empty;
  }


  uint64_t
  operator()(std::istream &is)
  {
    size_t bytes_read{ 0 };
    bd::Info() << "Starting reader loop.";

    while (true) {

      buffer_type *buf{ m_empty->pop() };
      if (!buf->getPtr()) {
        break;
      }

      is.read(reinterpret_cast<char *>(buf->getPtr()),
              buf->getMaxNumElements() * sizeof(Ty));

      std::streamsize amount{ is.gcount() };
      bytes_read += amount;

      buf->setNumElements(amount / sizeof(Ty));
      buf->setIndexOffset(bytes_read / sizeof(Ty));

      bd::Dbg() << "Reader read " << bytes_read << " bytes";
      m_full->push(buf);

      // entire file has been read.
      if (amount < static_cast<long long>(buf->getMaxNumElements())) {
        break;
      }

    }

    bd::Info() << "Reader loop finished.";

    m_full->push(new bd::Buffer<Ty>(nullptr, 0));

    return bytes_read;
  }


  static void
  start(Reader &r, std::ifstream &is)
  {
    r.reader_future =
        std::async([&r](std::ifstream &of) -> uint64_t {
                     return r(of);
                   },
                   std::ref(is));
  }


  uint64_t
  join()
  {
    reader_future.wait();
    return reader_future.get();
  }


private:
  queue_type *m_empty;
  queue_type *m_full;

  std::future<uint64_t> reader_future;

};


}


#endif // bd_reader_h
