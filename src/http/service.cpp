// HTTP服务器
// 实现业务服务类

#include "types.h"
#include "client.h"
#include "status.h"
#include "globals.h"
#include "service.h"

#define ROUTE_FILES "/files/"
#define APPID       "dsmvideo"
#define USERID      "dsm001"
#define FILE_SLICE  1024 * 1024 * 8LL

service_c::service_c(acl::socket_stream* conn, acl::session* sess)
    : HttpServlet(conn, sess) {
}

void setErrorResponse(acl::HttpServletResponse& res, int status, const char* message) {
    res.setStatus(status)
       .setContentType("text/html;charset=utf-8");
    res.getOutputStream().write(acl::string().format("<root error='%s' />\r\n", message));
}

bool service_c::doGet(acl::HttpServletRequest& req, acl::HttpServletResponse& res) {
    // 从请求中提取资源路径
    acl::string path = req.getPathInfo();
    logger("Received GET request, path: %s", path.c_str());

    if (!path.ncompare(ROUTE_FILES, strlen(ROUTE_FILES))) {
        // 处理文件路由
        files(req, res);
    } else {
        logger_error("Unknown route, path: %s", path.c_str());
        setErrorResponse(res, STATUS_BAD_REQUEST, "Unknown route");
    }

    // 发送响应
    return res.write(NULL, 0);
}

bool service_c::doPost(acl::HttpServletRequest& req, acl::HttpServletResponse& res) {
    setErrorResponse(res, STATUS_NOT_IMPLEMENTED, "POST method not supported");
    return res.write(NULL, 0);
}

bool service_c::doOptions(acl::HttpServletRequest& req, acl::HttpServletResponse& res) {
    res.setStatus(STATUS_OK)
       .setContentType("text/plain;charset=utf-8")
       .setContentLength(0)
       .setKeepAlive(req.isKeepAlive());

    // 发送响应
    return res.write(NULL, 0);
}

bool service_c::doError(acl::HttpServletRequest& req, acl::HttpServletResponse& res) {
    setErrorResponse(res, STATUS_BAD_REQUEST, "Some error happened");
    return true;
}

bool service_c::doOther(acl::HttpServletRequest& req, acl::HttpServletResponse& res, char const* method) {
    acl::string message;
    message.format("Unknown request method %s", method);
    setErrorResponse(res, STATUS_BAD_REQUEST, message.c_str());
    return true;
}

////////////////////////////////////////////////////////////////////////

// 处理文件路由
bool service_c::files(acl::HttpServletRequest& req, acl::HttpServletResponse& res) {
    res.setKeepAlive(req.isKeepAlive());

    acl::string path = req.getPathInfo();
    acl::string fileid = path.c_str() + strlen(ROUTE_FILES);

    // 记录提取的路径和文件 ID
    logger("Processing file request, path: %s, fileid: %s", path.c_str(), fileid.c_str());

    // 检查文件 ID 是否为空
    if (fileid.empty()) {
        setErrorResponse(res, STATUS_BAD_REQUEST, "File ID is empty");
        return false;
    }

    // URL 解码处理文件 ID
    //fileid = acl::string::urldecode(fileid.c_str());

    res.setContentType("application/octet-stream");
    res.setHeader("Content-Disposition", acl::string().format("attachment; filename=%s", fileid.c_str()).c_str());

    client_c client;
    long long filesize = 0;

    // 获取文件大小
    if (client.filesize(APPID, USERID, fileid, filesize) != OK) {
        setErrorResponse(res, STATUS_INTER_SERVER_ERROR, "Failed to retrieve file size");
        return false;
    }

    long long range_from = 0, range_to = filesize - 1;
    if (req.getRange(range_from, range_to)) {
        // 检查 Range 是否有效
        if (range_to >= filesize || range_from > range_to) {
            setErrorResponse(res, STATUS_BAD_REQUEST, "Invalid range");
            logger_warn("Invalid range: range_from=%lld, range_to=%lld, filesize=%lld",
                        range_from, range_to, filesize);
            return false;
        }
    } else {
        logger("No range header, sending full file: filesize=%lld", filesize);
    }

    // 设置响应头
    res.setHeader("Accept-Ranges", "bytes");
    res.setHeader("Content-Range", acl::string().format("bytes %lld-%lld/%lld", range_from, range_to, filesize).c_str());
    res.setHeader("Content-Length", std::to_string(range_to - range_from + 1).c_str());

    long long remain = range_to - range_from + 1;
    long long offset = range_from;
    char* downdata = nullptr;

    // 分块下载并发送文件数据
    while (remain > 0) {
        long long size = std::min(remain, FILE_SLICE);
        long long downsize = 0;

        if (client.download(APPID, USERID, fileid.c_str(), offset, size, &downdata, downsize) != OK) {
            free(downdata);
            setErrorResponse(res, STATUS_INTER_SERVER_ERROR, "Download failed");
            logger_error("Download failed: fileid=%s, offset=%lld, size=%lld", fileid.c_str(), offset, size);
            return false;
        }

        if (!res.write(downdata, downsize)) {
            free(downdata);
            setErrorResponse(res, STATUS_INTER_SERVER_ERROR, "Response write failed");
            logger_error("Response write failed: fileid=%s, offset=%lld, size=%lld", fileid.c_str(), offset, size);
            return false;
        }

        free(downdata);
        downdata = nullptr;

        remain -= downsize;
        offset += downsize;
    }

    logger("File transfer completed: fileid=%s, total_bytes=%lld", fileid.c_str(), range_to - range_from + 1);
    return true;
}