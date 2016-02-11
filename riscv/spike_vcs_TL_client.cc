/// \file
/// \copyright Syntacore 2015
/// See LICENSE for license details

#include "spike_vcs_TL.hxx"

namespace spike_vcs_TL {
std::shared_ptr<Request const>
Request::create(uint8_t a_sn, Request_type a_cmd, uint32_t a_address /*= 0*/, uint8_t a_size /*= 0*/, uint32_t a_data /*= 0*/) {
    typedef std::shared_ptr<Request const> result_type;
    return result_type(new Request(a_sn, a_cmd, a_address, a_size, a_data));
}
std::vector<uint8_t>
Request::serialize() const {
    typedef std::vector<uint8_t> res_type;
    res_type res;
    res.reserve(12);
    res.push_back(m_sn);
    res.push_back(static_cast<uint8_t>(m_cmd));
    switch (this->m_cmd) {
        case Request_type::read:
        case Request_type::csr_read:
            {
                res.resize(8, 0);
                res[2] = this->m_size;
                res[4] = static_cast<uint8_t>(this->m_address >> (8 * 3));
                res[5] = static_cast<uint8_t>(this->m_address >> (8 * 2));
                res[6] = static_cast<uint8_t>(this->m_address >> (8 * 1));
                res[7] = static_cast<uint8_t>(this->m_address >> (8 * 0));
            }
            break;
        case Request_type::write:
        case Request_type::csr_write:
            {
                res.resize(12, 0);
                res[2] = this->m_size;
                res[4] = static_cast<uint8_t>(this->m_address >> (8 * 3));
                res[5] = static_cast<uint8_t>(this->m_address >> (8 * 2));
                res[6] = static_cast<uint8_t>(this->m_address >> (8 * 1));
                res[7] = static_cast<uint8_t>(this->m_address >> (8 * 0));

                res[8] = static_cast<uint8_t>(this->m_data >> (8 * 3));
                res[9] = static_cast<uint8_t>(this->m_data >> (8 * 2));
                res[10] = static_cast<uint8_t>(this->m_data >> (8 * 1));
                res[11] = static_cast<uint8_t>(this->m_data >> (8 * 0));
            }
            break;
        case Request_type::irq_lines_set:
            {
                res.resize(8, 0);
                res[4] = static_cast<uint8_t>(this->m_address >> (8 * 3));
                res[5] = static_cast<uint8_t>(this->m_address >> (8 * 2));
                res[6] = static_cast<uint8_t>(this->m_address >> (8 * 1));
                res[7] = static_cast<uint8_t>(this->m_address >> (8 * 0));
            }
            break;
        default:
            break;
    }
    return res;
}
std::shared_ptr<ACK const>
ACK::deserialize(std::vector<uint8_t> const& a_buf) {
    typedef std::shared_ptr<ACK const> res_type;
    if (a_buf.size() < 8) {
        LOGGER << "a_buf.size() < 8 (" << a_buf.size() << ")" << std::endl;
        return res_type();
    }
    uint8_t const sn = a_buf[0];
    if (a_buf[1] >= static_cast<uint8_t>(Request_type::req_num)) {
        LOGGER << "a_buf[1] > static_cast<uint8_t>(Request_type::req_num) (" << a_buf[1] << ")" << std::endl;
        return res_type();
    }
    Request_type const cmd = static_cast<Request_type>(a_buf[1]);
    uint32_t const state = (((((a_buf[4] << 8) | a_buf[5]) << 8) | a_buf[6]) << 8) | a_buf[7];
    switch (cmd) {
        case Request_type::read:
        case Request_type::csr_read:
        case Request_type::reset_state:
        case Request_type::irq_lines_get:
            {
                if (a_buf.size() < 12) {
                    LOGGER << "a_buf.size() < 12 (" << a_buf.size() << ")" << std::endl;
                }
                uint32_t const data = (((((a_buf[8] << 8) | a_buf[9]) << 8) | a_buf[10]) << 8) | a_buf[11];
                return res_type(new ACK(sn, cmd, state, data));
            }
            break;
        default:
            return res_type(new ACK(sn, cmd, state));
    }
}
Client& Client::instance()
{
    static Client _instance;
    return _instance;
}
}  // namespace spike_vcs_TL
