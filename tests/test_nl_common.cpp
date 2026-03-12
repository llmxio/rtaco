#include <gtest/gtest.h>
#include <array>
#include <cstring>
#include <vector>

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

TEST(NLCommonTest, CheckedNlmsgerrFromHeader) {
    std::vector<uint8_t> buf(NLMSG_SPACE(sizeof(nlmsgerr)), 0);

    auto* header = reinterpret_cast<nlmsghdr*>(buf.data());
    header->nlmsg_len = NLMSG_LENGTH(sizeof(nlmsgerr));
    header->nlmsg_type = NLMSG_ERROR;

    auto* error = reinterpret_cast<nlmsgerr*>(NLMSG_DATA(header));
    error->error = -EEXIST;

    const auto* extracted = checked_nlmsgerr(*header);
    ASSERT_NE(extracted, nullptr);
    EXPECT_EQ(extracted->error, -EEXIST);
}

TEST(NLCommonTest, CheckedAttrBeginAndTraversal) {
    constexpr uint32_t kPayload = 42;
    constexpr size_t kAttrLen = RTA_LENGTH(sizeof(kPayload));
    constexpr size_t kMsgLen = NLMSG_LENGTH(sizeof(ifinfomsg)) + RTA_ALIGN(kAttrLen);
    struct alignas(nlmsghdr) MsgBuffer {
        std::array<uint8_t, kMsgLen> bytes{};
    } buffer;

    auto* header = reinterpret_cast<nlmsghdr*>(buffer.bytes.data());
    header->nlmsg_len = static_cast<uint32_t>(kMsgLen);
    auto* info = reinterpret_cast<ifinfomsg*>(NLMSG_DATA(header));

    int attr_length = 0;
    const rtattr* attr = checked_attr_begin(*header, info, attr_length);
    ASSERT_NE(attr, nullptr);
    EXPECT_GT(attr_length, 0);

    auto* mutable_attr = const_cast<rtattr*>(attr);
    mutable_attr->rta_len = static_cast<unsigned short>(kAttrLen);
    mutable_attr->rta_type = IFLA_MTU;
    std::memcpy(RTA_DATA(mutable_attr), &kPayload, sizeof(kPayload));

    EXPECT_TRUE(checked_attr_ok(attr, attr_length));
    EXPECT_EQ(attr->rta_type, IFLA_MTU);
}

TEST(NLCommonTest, CheckedPayloadAccess) {
    constexpr uint32_t kValue = 1337;
    const size_t attr_len = RTA_LENGTH(sizeof(kValue));
    std::vector<uint8_t> buf(attr_len, 0);

    auto* attr = reinterpret_cast<rtattr*>(buf.data());
    attr->rta_len = static_cast<unsigned short>(attr_len);
    std::memcpy(RTA_DATA(attr), &kValue, sizeof(kValue));

    const auto* value = checked_payload<uint32_t>(*attr);
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, kValue);

    attr->rta_len = RTA_LENGTH(sizeof(uint16_t));
    EXPECT_EQ(checked_payload<uint32_t>(*attr), nullptr);
}

TEST(NLCommonTest, CheckedPayloadSupportsUnalignedData) {
    constexpr uint64_t kValue = 0x0102030405060708ULL;
    const size_t attr_len = RTA_LENGTH(sizeof(kValue));
    struct alignas(uint64_t) AlignedStorage {
        std::array<uint8_t, attr_len> bytes{};
    } storage;

    auto* attr = reinterpret_cast<rtattr*>(storage.bytes.data());
    attr->rta_len = static_cast<unsigned short>(attr_len);
    attr->rta_type = IFLA_MTU;

    std::memcpy(RTA_DATA(attr), &kValue, sizeof(kValue));

    const auto payload_addr = reinterpret_cast<std::uintptr_t>(RTA_DATA(attr));
    ASSERT_NE((payload_addr % alignof(uint64_t)), 0);

    const auto value = checked_payload_copy<uint64_t>(*attr);
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, kValue);
}
