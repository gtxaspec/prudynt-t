#ifndef RTSP_STATUS_HPP
#define RTSP_STATUS_HPP

#include <string>
#include <map>
#include <mutex>
#include <filesystem>
#include <vector>

/**
 * RTSPStatus - Runtime RTSP stream parameter exposure interface
 *
 * Provides access to active RTSP stream parameters through a file-based interface
 * similar to /proc filesystem pattern. This enables shell scripts and external tools
 * to query current streaming configuration without parsing JSON files or interacting
 * with the application directly.
 *
 * Interface Location: /run/prudynt/rtsp/
 *
 * For each active stream (stream0, stream1), creates files:
 * - /run/prudynt/rtsp/stream0/format      (e.g., "H264")
 * - /run/prudynt/rtsp/stream0/fps         (e.g., "25")
 * - /run/prudynt/rtsp/stream0/width       (e.g., "1920")
 * - /run/prudynt/rtsp/stream0/height      (e.g., "1080")
 * - /run/prudynt/rtsp/stream0/endpoint    (e.g., "ch0")
 * - /run/prudynt/rtsp/stream0/url         (e.g., "rtsp://192.168.1.100:554/ch0")
 * - /run/prudynt/rtsp/stream0/bitrate     (e.g., "3000")
 * - /run/prudynt/rtsp/stream0/mode        (e.g., "CBR")
 *
 * Usage:
 * - Call updateStreamStatus() when streams are created/updated
 * - Call removeStreamStatus() when streams are stopped
 * - Files are automatically created/removed as needed
 */
class RTSPStatus {
public:
    struct StreamInfo {
        std::string format;         // Video codec (H264, H265)
        int fps;                    // Frame rate
        int width;                  // Video width
        int height;                 // Video height
        std::string endpoint;       // RTSP endpoint (ch0, ch1, etc.)
        std::string url;            // Full RTSP URL
        int bitrate;                // Bitrate in kbps
        std::string mode;           // Bitrate mode (CBR, VBR)
        bool enabled;               // Stream enabled status

        StreamInfo() : fps(0), width(0), height(0), bitrate(0), enabled(false) {}
    };

    /**
     * Initialize the RTSP status interface
     * Creates the base directory structure
     * @return true if initialization successful
     */
    static bool initialize();

    /**
     * Clean up the RTSP status interface
     * Removes all status files and directories
     */
    static void cleanup();

    /**
     * Update status for a specific stream
     * @param streamName Stream identifier (e.g., "stream0", "stream1")
     * @param info Stream information to expose
     * @return true if update successful
     */
    static bool updateStreamStatus(const std::string& streamName, const StreamInfo& info);

    // Write a custom parameter file under a given stream directory (e.g., "audio0")
    static bool writeCustomParameter(const std::string& streamName,
                                     const std::string& parameter,
                                     const std::string& value);

    /**
     * Remove status for a specific stream
     * @param streamName Stream identifier to remove
     * @return true if removal successful
     */
    static bool removeStreamStatus(const std::string& streamName);

    /**
     * Get current status for a stream (for internal use)
     * @param streamName Stream identifier
     * @return StreamInfo structure, empty if not found
     */
    static StreamInfo getStreamStatus(const std::string& streamName);

    /**
     * Check if the status interface is available
     * @return true if interface is initialized and accessible
     */
    static bool isAvailable();

    /**
     * Get list of active streams
     * @return vector of active stream names
     */
    static std::vector<std::string> getActiveStreams();

private:
    static const std::string STATUS_BASE_DIR;
    static std::mutex statusMutex;
    static std::map<std::string, StreamInfo> activeStreams;

    /**
     * Create directory structure for a stream
     * @param streamName Stream identifier
     * @return true if creation successful
     */
    static bool createStreamDirectory(const std::string& streamName);

    /**
     * Write a parameter file for a stream
     * @param streamName Stream identifier
     * @param parameter Parameter name (e.g., "format", "fps")
     * @param value Parameter value as string
     * @return true if write successful
     */
    static bool writeParameter(const std::string& streamName,
                              const std::string& parameter,
                              const std::string& value);

    /**
     * Remove directory and all files for a stream
     * @param streamName Stream identifier
     * @return true if removal successful
     */
    static bool removeStreamDirectory(const std::string& streamName);

    /**
     * Ensure base directory exists
     * @return true if directory exists or was created successfully
     */
    static bool ensureBaseDirectory();
};

#endif // RTSP_STATUS_HPP
