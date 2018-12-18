#include <iostream>
#include <fstream>

namespace NTypeList {

//////////////////////////////////////////////////////////

template<typename ... Args>
struct TTypeList {};

template<typename Head, typename ... Tail>
struct TTypeList<Head, Tail...> {
    typedef Head head;
    typedef TTypeList<Tail...> tail;
};

//////////////////////////////////////////////////////////

}   // NTypeList

namespace {

using namespace NTypeList;

//////////////////////////////////////////////////////////

class IDecoder {
public:
    virtual void Decompress(const void* fromBuffer, void* toBuffer) const = 0;
};

class TNoDecompressor: IDecoder {
public:
    void Decompress(const void*, void*) const
    {}
};

//////////////////////////////////////////////////////////

template<typename Types>
struct TCalculateSize
    : std::integral_constant<
        size_t,
        sizeof(typename Types::head) + TCalculateSize<typename Types::tail>::value
    > {
};

template<>
struct TCalculateSize<TTypeList<>>
    : std::integral_constant<size_t, 0> {
};

//////////////////////////////////////////////////////////

struct TNoneType {};

std::istream& operator>>(std::istream& is, TNoneType&)
{
    return is;
}

template<typename Types, typename CompressedTypes, typename Decoders>
class TReaderImpl {
private:
    std::ifstream& In_;
    TReaderImpl<typename Types::tail, typename CompressedTypes::tail, typename Decoders::tail> TailReader_;

public:
    TReaderImpl(std::ifstream& in)
        : In_(in)
        , TailReader_(In_)
    {}

    void ReadNextLine(void* buffer)
    {
        if (std::is_same<typename Decoders::head, TNoDecompressor>::value) {
            In_ >> (*static_cast<typename Types::head*>(buffer));
        } else {
            void* compressedBuffer = malloc(sizeof(typename CompressedTypes::head));
            In_ >> (*static_cast<typename CompressedTypes::head*>(compressedBuffer));

            auto decoder = new typename Decoders::head;
            decoder->Decompress(compressedBuffer, buffer);

            delete decoder;

            free(compressedBuffer);
        }

        TailReader_.ReadNextLine(static_cast<char*>(buffer) + sizeof(typename Types::head));
    }
};

template<>
class TReaderImpl<TTypeList<>, TTypeList<>, TTypeList<>> {
public:
    TReaderImpl(std::ifstream& in)
    {}

    void ReadNextLine(void* buffer)
    {}
};

template<typename Types, typename CompressedTypes, typename Decoders>
class TReader {
private:
    std::ifstream In_;
    TReaderImpl<Types, CompressedTypes, Decoders> Impl_;

public:
    TReader(const std::string& fileName)
        : In_(fileName)
        , Impl_(In_)
    {}

public:
    void* ReadNextLine()
    {
        void* buffer = malloc(TCalculateSize<Types>::value);
        Impl_.ReadNextLine(buffer);
        return buffer;
    }
};

//////////////////////////////////////////////////////////

}   // namespace

class TIntToDoubleDecoder: IDecoder {
public:
    virtual void Decompress(const void* fromBuffer, void* toBuffer) const
    {
        auto fromValue = static_cast<double>(*static_cast<const int*>(fromBuffer));
        std::memcpy(toBuffer, &fromValue, sizeof(fromValue));
    }
};

int main()
{
    auto reader = TReader<
            TTypeList<double, std::string>,
            TTypeList<int, TNoneType>,
            TTypeList<TIntToDoubleDecoder, TNoDecompressor>>
        ("input.txt");

    for (int i = 0; i < 3; ++i) {
        void* data = reader.ReadNextLine();

        auto doubleValue = (*static_cast<double*>(data));
        auto strValue = (*reinterpret_cast<std::string*>(static_cast<char*>(data) + sizeof(doubleValue)));

        std::cout.precision(5);
        std::cout.setf(std::ios::fixed, std::ios::floatfield);
        std::cout << doubleValue << " " << strValue << std::endl;

        free(data);
    }

    return 0;
}