#include "codec.h"

#include <muduo/base/Logging.h>
#include <muduo/net/Endian.h>
#include <google-inl.h>

#include <google/protobuf/descriptor.h>

#include <zlib.h>

using namespace muduo;
using namespace muduo::net;

namespace
{
    const string kNoErrorStr = "NoError";
    const string kInvalidLengthStr = "InvalidLength";
    const string kCheckSumErrorStr = "CheckSumError";
    const string kInvalidNameLenStr = "InvalidNameLen";
    const string kUnknownMessageTypeStr = "UnknownMessageType";
    const string kParseErrorStr = "ParseError";
    const string kUnknownErrorStr = "UnknownError";
}

const string &ProtobufCodec::errorCodeToString(ErrorCode errorCode)
{
    switch (errorCode)
    {
    case kNoError:
        return kNoErrorStr;
    case kInvalidLength:
        return kInvalidLengthStr;
    case kCheckSumError:
        return kCheckSumErrorStr;
    case kInvalidNameLen:
        return kInvalidNameLenStr;
    case kUnknownMessageType:
        return kUnknownMessageTypeStr;
    case kParseError:
        return kParseErrorStr;
    default:
        return kUnknownErrorStr;
    }
}

void ProtobufCodec::defaultErrorCallback(const muduo::net::TcpConnectionPtr &conn,
                                         muduo::net::Buffer *buf,
                                         muduo::Timestamp,
                                         ErrorCode errorCode)
{
    LOG_ERROR << "ProtobufCodec::defaultErrorCallback - " << errorCodeToString(errorCode);
    if (conn && conn->connected())
    {
        conn->shutdown();
    }
}

int32_t asInt32(const char *buf)
{
    int32_t be32 = 0;
    ::memcpy(&be32, buf, sizeof(be32));
    return sockets::networkToHost32(be32);
}

void ProtobufCodec::onMessage(const TcpConnectionPtr &conn,
                              Buffer *buf,
                              Timestamp receiveTime)
{
    while (buf->readableBytes() >= kMinMessageLen + kHeaderLen)
    {
        const int32_t len = buf->peekInt32();
        if (len > kMaxMessageLen || len < kMinMessageLen)
        {
            errorCallback_(conn, buf, receiveTime, kInvalidLength);
            break;
        }
        else if (buf->readableBytes() >= implicit_cast<size_t>(len + kHeaderLen))
        {
            ErrorCode errorCode = kNoError;
            MessagePtr message = parse(buf->peek() + kHeaderLen, len, &errorCode);
            if (errorCode == kNoError && message)
            {
                messageCallback_(conn, message, receiveTime);
                buf->retrieve(kHeaderLen + len);
            }
            else
            {
                errorCallback_(conn, buf, receiveTime, errorCode);
                break;
            }
        }
        else
        {
            break;
        }
    }
}

google::protobuf::Message *ProtobufCodec::createMessage(const std::string &typeName)
{
    google::protobuf::Message *message = NULL;
    const google::protobuf::Descriptor *descriptor =
        google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(typeName);
    if (descriptor)
    {
        const google::protobuf::Message *prototype =
            google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
        if (prototype)
        {
            message = prototype->New();
        }
    }
    return message;
}

MessagePtr ProtobufCodec::parse(const char *buf, int len, ErrorCode *error)
{
    MessagePtr message;

    // crc
    int32_t expectedCheckSum = asInt32(buf + len - kHeaderLen);
    int32_t checkSum = static_cast<int32_t>(
        ::crc32(1,
                reinterpret_cast<const Bytef *>(buf),
                static_cast<int>(len - kHeaderLen)));
    if (checkSum == expectedCheckSum)
    {
        // get message type name
        int32_t nameLen = asInt32(buf);
        if (nameLen >= 2 && nameLen <= len - 2 * kHeaderLen)
        {
            std::string typeName(buf + kHeaderLen, buf + kHeaderLen + nameLen - 1); // 去掉‘\0’
            // create message object
            message.reset(createMessage(typeName));
            if (message)
            {
                // parse from buffer
                const char *data = buf + kHeaderLen + nameLen;
                int32_t dataLen = len - nameLen - 2 * kHeaderLen;
                if (message->ParseFromArray(data, dataLen))
                {
                    *error = kNoError;
                }
                else
                {
                    *error = kParseError;
                }
            }
            else
            {
                *error = kUnknownMessageType;
            }
        }
        else
        {
            *error = kInvalidNameLen;
        }
    }
    else
    {
        *error = kCheckSumError;
    }

    return message;
}

void ProtobufCodec::send(const muduo::net::TcpConnectionPtr &conn,
                         const google::protobuf::Message &message)
{
    muduo::net::Buffer buf;
    fillbuffer(&buf, message);
    conn->send(&buf);
}

void ProtobufCodec::fillbuffer(muduo::net::Buffer *buf, const google::protobuf::Message &message)
{
    const std::string &type_name = message.GetTypeName();
    int32_t namelen = type_name.size() + 1;
    buf->appendInt32(namelen);
    buf->append(type_name.c_str(), namelen);

    int len = google::protobuf::internal::ToIntSize(message.ByteSizeLong());
    buf->ensureWritableBytes(len);
    uint8_t *begin = reinterpret_cast<uint8_t *>(buf->beginWrite());
    uint8_t *end = message.SerializeWithCachedSizesToArray(begin);
    if ((end - begin) != len)
    {
        ByteSizeConsistencyError(len, end - begin, len);
    }
    buf->hasWritten(len);

    int32_t crc = ::crc32(1,
                          reinterpret_cast<const Bytef *>(buf->peek()),
                          static_cast<uInt>(buf->readableBytes()));
    buf->appendInt32(crc);
    int32_t lenth = static_cast<int32_t>(buf->readableBytes());
    buf->prependInt32(lenth);
}