// StreamUtils - github.com/bblanchon/ArduinoStreamUtils
// Copyright Benoit Blanchon 2019
// MIT License

#include "FailingAllocator.hpp"

#include "StreamUtils/Streams/MemoryStream.hpp"
#include "StreamUtils/Streams/ReadBufferingStream.hpp"
#include "StreamUtils/Streams/SpyingStream.hpp"

#include "doctest.h"

using namespace StreamUtils;

TEST_CASE("ReadBufferingStream") {
  MemoryStream upstream(64);
  MemoryStream history(64);
  SpyingStream spy{upstream, history};

  SUBCASE("capacity = 4") {
    ReadBufferingStream bufferedStream{spy, 4};
    Stream& stream = bufferedStream;

    SUBCASE("available()") {
      upstream.print("ABCDEFGH");

      SUBCASE("empty input") {
        upstream.flush();
        CHECK(stream.available() == 0);
        CHECK(history.readString() == "available() -> 0");
      }

      SUBCASE("read empty input") {
        upstream.flush();

        int n = stream.read();

        CHECK(n == -1);
        CHECK(stream.available() == 0);
#if STREAMUTILS_STREAM_READBYTES_IS_VIRTUAL
        CHECK(history.readString() ==
              "readBytes(4) -> 0"
              "available() -> 0");
#endif
      }

      SUBCASE("same a upstream") {
        CHECK(stream.available() == 8);
        CHECK(history.readString() == "available() -> 8");
      }

      SUBCASE("upstream + in buffer") {
        int n = stream.read();

        CHECK(n == 'A');
        CHECK(stream.available() == 7);
#if STREAMUTILS_STREAM_READBYTES_IS_VIRTUAL
        CHECK(history.readString() ==
              "readBytes(4) -> 4"
              "available() -> 4");
#endif
      }
    }

    SUBCASE("peek()") {
      SUBCASE("returns -1 when empty") {
        upstream.flush();

        int result = stream.peek();

        CHECK(result == -1);
        CHECK(history.readString() == "peek() -> -1");
      }

      SUBCASE("doesn't call readBytes() when buffer is empty") {
        upstream.print("A");

        int result = stream.peek();

        CHECK(result == 'A');
        CHECK(history.readString() == "peek() -> 65");
      }

      SUBCASE("doesn't call peek() when buffer is full") {
        upstream.print("AB");

        stream.read();
        int result = stream.peek();

        CHECK(result == 'B');
#if STREAMUTILS_STREAM_READBYTES_IS_VIRTUAL
        CHECK(history.readString() == "readBytes(4) -> 2");
#endif
      }
    }

    SUBCASE("read()") {
      SUBCASE("reads 4 bytes at a time") {
        upstream.print("ABCDEFG");
        std::string result;

        for (int i = 0; i < 7; i++) {
          result += (char)stream.read();
        }

        CHECK(result == "ABCDEFG");
#if STREAMUTILS_STREAM_READBYTES_IS_VIRTUAL
        CHECK(history.readString() ==
              "readBytes(4) -> 4"
              "readBytes(4) -> 3");
#endif
      }

      SUBCASE("returns -1 when empty") {
        upstream.flush();

        int result = stream.read();

        CHECK(result == -1);
#if STREAMUTILS_STREAM_READBYTES_IS_VIRTUAL
        CHECK(history.readString() == "readBytes(4) -> 0");
#endif
      }
    }

    SUBCASE("readBytes()") {
      SUBCASE("empty input") {
        upstream.flush();

        char c;
        size_t result = stream.readBytes(&c, 1);

        CHECK(result == 0);
#if STREAMUTILS_STREAM_READBYTES_IS_VIRTUAL
        CHECK(history.readString() == "readBytes(4) -> 0");
#endif
      }

      SUBCASE("reads 4 bytes when requested one") {
        upstream.print("ABCDEFG");

        char c;
        size_t result = stream.readBytes(&c, 1);

        CHECK(c == 'A');
        CHECK(result == 1);
#if STREAMUTILS_STREAM_READBYTES_IS_VIRTUAL
        CHECK(history.readString() == "readBytes(4) -> 4");
#endif
      }

      SUBCASE("copy one byte from buffer") {
        upstream.print("ABCDEFGH");
        stream.read();  // load buffer

        char c;
        size_t result = stream.readBytes(&c, 1);

        CHECK(c == 'B');
        CHECK(result == 1);
#if STREAMUTILS_STREAM_READBYTES_IS_VIRTUAL
        CHECK(history.readString() == "readBytes(4) -> 4");
#endif
      }

      SUBCASE("copy content from buffer then bypass buffer") {
        upstream.print("ABCDEFGH");
        stream.read();  // load buffer

        char c[8] = {0};
        size_t result = stream.readBytes(c, 7);

        CHECK(c == std::string("BCDEFGH"));
        CHECK(result == 7);
#if STREAMUTILS_STREAM_READBYTES_IS_VIRTUAL
        CHECK(history.readString() ==
              "readBytes(4) -> 4"
              "readBytes(4) -> 4");
#endif
      }

      SUBCASE("copy content from buffer twice") {
        upstream.print("ABCDEFGH");
        stream.read();  // load buffer

        char c[8] = {0};
        size_t result = stream.readBytes(c, 4);

        CHECK(c == std::string("BCDE"));
        CHECK(result == 4);
#if STREAMUTILS_STREAM_READBYTES_IS_VIRTUAL
        CHECK(history.readString() ==
              "readBytes(4) -> 4"
              "readBytes(4) -> 4");
#endif
      }

      SUBCASE("read past the end") {
        upstream.print("A");

        char c;
        stream.readBytes(&c, 1);
        size_t result = stream.readBytes(&c, 1);

        CHECK(result == 0);
#if STREAMUTILS_STREAM_READBYTES_IS_VIRTUAL
        CHECK(history.readString() ==
              "readBytes(4) -> 1"
              "readBytes(4) -> 0");
#endif
      }
    }

    SUBCASE("flush()") {
      stream.flush();
      CHECK(history.readString() == "flush()");
    }

    SUBCASE("copy constructor") {
      upstream.print("ABCDEFGH");
      bufferedStream.read();

      auto dup = bufferedStream;

      int result = dup.read();

      CHECK(result == 'B');
#if STREAMUTILS_STREAM_READBYTES_IS_VIRTUAL
      CHECK(history.readString() == "readBytes(4) -> 4");
#endif
    }
  }

  SUBCASE("No memory") {
    BasicReadBufferingStream<FailingAllocator> stream(spy, 4);

    SUBCASE("available()") {
      upstream.print("ABC");

      CHECK(stream.available() == 3);
    }

    // SUBCASE("capacity()") {
    //   CHECK(stream.capacity() == 0);
    // }

    SUBCASE("peek()") {
      upstream.print("ABC");

      int c = stream.peek();

      CHECK(c == 'A');
      CHECK(history.readString() == "peek() -> 65");
    }

    SUBCASE("read()") {
      upstream.print("ABC");

      int c = stream.read();

      CHECK(c == 'A');
      CHECK(history.readString() == "read() -> 65");
    }

    SUBCASE("readBytes()") {
      upstream.print("ABC");

      char s[4] = {0};
      int n = stream.readBytes(s, 3);

      CHECK(n == 3);
      CHECK(s == std::string("ABC"));
#if STREAMUTILS_STREAM_READBYTES_IS_VIRTUAL
      CHECK(history.readString() == "readBytes(3) -> 3");
#endif
    }
  }

  SUBCASE("Real example") {
    ReadBufferingStream bufferedStream{spy, 64};
    Stream& stream = bufferedStream;

    upstream.print("{\"helloWorld\":\"Hello World\"}");

    char c[] = "ABCDEFGH";
    CHECK(stream.readBytes(&c[0], 1) == 1);
    CHECK(stream.readBytes(&c[1], 1) == 1);
    CHECK(stream.readBytes(&c[2], 1) == 1);
    CHECK(stream.readBytes(&c[3], 1) == 1);

    CHECK(c == std::string("{\"heEFGH"));
#if STREAMUTILS_STREAM_READBYTES_IS_VIRTUAL
    CHECK(history.readString() == "readBytes(64) -> 28");
#endif
  }
}
