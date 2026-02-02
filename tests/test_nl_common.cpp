#include <gtest/gtest.h>
#include <cstring>

#include "rtaco/core/nl_common.hxx"

using namespace llmx::rtaco;

TEST(NLCommonTest, TrimStringAndAttributeString) {
    // trim_string should remove trailing nulls
    std::string_view sv = "abc\0\0";
    auto trimmed = trim_string(sv);
    EXPECT_EQ(trimmed, "abc");

    // Build an rtattr with payload string
    const char* payload = "eth0";
    const size_t payload_len = std::strlen(payload) + 1;
    const size_t attr_len = RTA_LENGTH(payload_len);
    std::vector<uint8_t> buf(attr_len);
    std::memset(buf.data(), 0, buf.size());

    auto attr = reinterpret_cast<rtattr*>(buf.data());
    attr->rta_len = static_cast<unsigned short>(RTA_LENGTH(payload_len));
    attr->rta_type = IFLA_IFNAME;
    std::memcpy(RTA_DATA(attr), payload, payload_len);

    auto s = attribute_string(*attr);
    EXPECT_EQ(s, "eth0");
}

TEST(NLCommonTest, GetMsgPayloadShort) {
    nlmsghdr short_hdr{};
    short_hdr.nlmsg_len = NLMSG_LENGTH(sizeof(ifinfomsg)) - 1; // too small

    auto ptr = get_msg_payload<ifinfomsg>(short_hdr);
    EXPECT_EQ(ptr, nullptr);
}
