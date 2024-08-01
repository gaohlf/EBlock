//
// Created by 20075 on 2024/6/19.
//

#include "S3Client.h"
#include "Log4Eblock.h"
#include <iostream>

void S3Client::showListObjects(void)
{
    // 列出存储桶中的对象
    Aws::S3::Model::ListObjectsRequest objects_request;
    objects_request.WithBucket(bucketname);

    auto list_objects_outcome = s3_client->ListObjects(objects_request);

    if (list_objects_outcome.IsSuccess()) {
        ELOG("Objects in bucket++++++++++ '%s':\n", bucketname.c_str());;

        Aws::Vector<Aws::S3::Model::Object> object_list =
                list_objects_outcome.GetResult().GetContents();

        for (auto const &s3_object : object_list) {
            ELOG("* %s\n", s3_object.GetKey().c_str());
        }
    } else {
        ELOG("ListObjects error: %s-%s\n",
             list_objects_outcome.GetError().GetExceptionName().c_str(),
             list_objects_outcome.GetError().GetMessage().c_str());
    }

}

S3Client::S3Client() : credentials("89XZEQZH85XWDB76X5WI", "9pqKll45TlKo6Lxj3eLzABqaMKXVnDobmE9H9CbC"),
                       s3_client(nullptr), endpoint("172.38.182.67:7480"), bucketname("222")
{
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        Aws::Client::ClientConfiguration clientConfig;
        // 创建自定义端点的客户端配置
        clientConfig.scheme = Aws::Http::Scheme::HTTP;  // 或者HTTPS
        clientConfig.endpointOverride = endpoint; // 例如: "localhost:9000" 或 "s3.yourdomain.com"
        s3_client = new Aws::S3::S3Client(credentials, nullptr, clientConfig);
    }
}

S3Client::~S3Client() {
    Aws::ShutdownAPI(options);
}

//上传指定名称对象的指定offset
bool S3Client::putObject(std::string name, unsigned long length, const char *buff) {
    Aws::S3::Model::PutObjectRequest request;
    request.SetBucket(bucketname);
    request.SetKey(name);

    auto data = Aws::MakeShared<Aws::StringStream>("");
    data->write(buff, length);
    request.SetBody(data);

    auto put_object_outcome = s3_client->PutObject(request);

    if (put_object_outcome.IsSuccess()) {
        ELOG("Successfully uploaded object to S3.\n");
        return true;
    } else {
        ELOG("PutObject error: %s - %s\n",
                  put_object_outcome.GetError().GetExceptionName().c_str(),
                  put_object_outcome.GetError().GetMessage().c_str());
        return false;
    }
}

bool S3Client::getObject(std::string name,unsigned long off, unsigned long length, char *buff) {
    // 创建GetObjectRequest
    Aws::S3::Model::GetObjectRequest request;
    request.SetBucket(bucketname);
    request.SetKey(name);
    std::string range = "bytes="+ std::to_string(off) + "-" + std::to_string(off+length);
    request.SetRange("bytes=0-99");
    // 获取对象
    auto get_object_outcome = s3_client->GetObject(request);

    if (get_object_outcome.IsSuccess()) {

        auto& retrieved_file = get_object_outcome.GetResultWithOwnership().GetBody();
        std::stringstream bufferInBody;
        bufferInBody << retrieved_file.rdbuf();
        std::string object_data = bufferInBody.str();
        size_t buffer_size = object_data.size();
        if(buffer_size != length)
        {
            ELOG("GetObject error: %buffer_size (%d) != length (%lu)\n",
                 buffer_size ,length);
            return false;
        }

        memcpy(buff, object_data.c_str(), length);

        ELOG("Successfully downloaded object from S3.\n");
        return true;
    }
    else
    {
        ELOG("GetObject error: %s - %s\n",
             get_object_outcome.GetError().GetExceptionName().c_str(),
             get_object_outcome.GetError().GetMessage().c_str() );
        return false;
    }

}
