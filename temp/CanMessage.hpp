/**
 * @file CanMessage.hpp
 * @brief Can message base class
 *
 */
#ifndef CAN_MESSAGE_HPP
#define CAN_MESSAGE_HPP

#include "CanMessageDefinitions.hpp"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <linux/can.h>
#include <memory>
#include <rxcpp/rx.hpp>
#include <span>

#if __cplusplus >= 202110L
// C++20 or later
#include <bit> //std::byteswap
using byteswap = std::byteswap;
#else
// C++ standard is older than C++20
// Implement our own byteswap
#include "CanUtilties.hpp"
#endif

constexpr size_t MAX_CAN_FRAME_SIZE = 8;
constexpr uint8_t TOTAL_BITS = 8;
constexpr uint64_t CONST_LONG_LONG_8 = 8ULL;
constexpr uint64_t CONST_LONG_LONG_7 = 7ULL;
constexpr uint64_t CONST_LONG_LONG_1 = 1ULL;
constexpr uint64_t CONST_LONG_LONG_64 = 64ULL;
constexpr uint8_t TOTAL_BYTES_ZERO_BASE = 7;
constexpr uint8_t TOTAL_BYTES = 8;
constexpr uint8_t CAN_FRAME_0 = 0;
constexpr uint8_t CAN_FRAME_1 = 1;
constexpr uint8_t CAN_FRAME_2 = 2;
constexpr uint8_t CAN_FRAME_3 = 3;
constexpr uint8_t CAN_FRAME_4 = 4;
constexpr uint8_t CAN_FRAME_5 = 5;
constexpr uint8_t CAN_FRAME_6 = 6;
constexpr uint8_t CAN_FRAME_7 = 7;

constexpr uint8_t BIT_POSITION_0 = 0;
constexpr uint8_t BIT_POSITION_1 = 1;
constexpr uint8_t BIT_POSITION_2 = 2;
constexpr uint8_t BIT_POSITION_3 = 3;
constexpr uint8_t BIT_POSITION_4 = 4;
constexpr uint8_t BIT_POSITION_5 = 5;
constexpr uint8_t BIT_POSITION_6 = 6;
constexpr uint8_t BIT_POSITION_7 = 7;
constexpr uint8_t BIT_POSITION_8 = 8;
constexpr uint8_t BIT_POSITION_9 = 9;
constexpr uint8_t BIT_POSITION_10 = 10;
constexpr uint8_t BIT_POSITION_11 = 11;
constexpr uint8_t BIT_POSITION_12 = 12;
constexpr uint8_t BIT_POSITION_13 = 13;
constexpr uint8_t BIT_POSITION_14 = 14;
constexpr uint8_t BIT_POSITION_15 = 15;
constexpr uint8_t BIT_POSITION_16 = 16;
constexpr uint8_t BIT_POSITION_17 = 17;
constexpr uint8_t BIT_POSITION_18 = 18;
constexpr uint8_t BIT_POSITION_19 = 19;
constexpr uint8_t BIT_POSITION_20 = 20;
constexpr uint8_t BIT_POSITION_21 = 21;
constexpr uint8_t BIT_POSITION_22 = 22;
constexpr uint8_t BIT_POSITION_23 = 23;
constexpr uint8_t BIT_POSITION_24 = 24;
constexpr uint8_t BIT_POSITION_25 = 25;
constexpr uint8_t BIT_POSITION_26 = 26;
constexpr uint8_t BIT_POSITION_27 = 27;
constexpr uint8_t BIT_POSITION_28 = 28;
constexpr uint8_t BIT_POSITION_29 = 29;
constexpr uint8_t BIT_POSITION_30 = 30;
constexpr uint8_t BIT_POSITION_31 = 31;
// Overflow is 32 to account for big endian messages
constexpr uint8_t BIT_POSITION_OVERFLOW = 31;

/**
 * @brief Data Format type of the payload data.
 *        Assumption:
 *        Little Endian Payload of (0x1234) needs to convert to (0x3412)
 *        Big Endian Leave alone.
 */
enum class CanFormatType {
  big_endian = 0,
  little_endian = 1,
};

/**
 * @brief Can message base class
 *
 */
class CanMessageBase {
private:
  /**
   * @brief Holder for can data
   *
   */
  can_frame frame_data;
  /**
   * @brief Holder for can ID
   *
   */
  CANID Id;

  std::array<uint8_t, MAX_CAN_FRAME_SIZE> muxIdArray;

public:
  CanMessageBase( const can_frame &frame, CANID _Id, const std::array<uint8_t, MAX_CAN_FRAME_SIZE> &muxArray )
      : frame_data( frame ), Id( _Id ), muxIdArray( muxArray ){};

  CanMessageBase( CANID _Id, const std::array<uint8_t, MAX_CAN_FRAME_SIZE> &muxArray )
      : frame_data( createCanFrame( _Id, muxArray[0] ) ), Id( _Id ), muxIdArray( muxArray ){};

  /**
   * @brief Create a Can Frame object
   *
   * @param id CAN ID Enumeration
   * @param can_data Data payload
   * @return can_frame
   */
  static auto createCanFrame( CANID id, uint8_t mux ) -> can_frame {
    can_frame frame{};
    frame.can_id = static_cast<canid_t>( id ) | CAN_EFF_FLAG;
    frame.can_dlc = MAX_CAN_FRAME_SIZE;
    auto frameData = std::span{ frame.data };
    std::fill( frameData.begin(), frameData.end(), 0 );
    frame.data[0] = mux;
    return frame;
  }


  /**
   * @brief Get the data from the CAN Message
   * NOTE: For Debugging purposes only
   *
   */
  [[nodiscard]] auto getData() const -> std::vector<uint8_t> {
    return std::vector<uint8_t>(        // NOLINT
        this->frame_data                // NOLINT
            .data,                      // NOLINT
        this->frame_data.data +         // NOLINT
            this->frame_data.can_dlc ); // NOLINT
  }

   /** NOTE: For Debugging purposes only
   * @return uint8_t
   */
  [[nodiscard]] auto getLength() const -> uint8_t { return this->frame_data.can_dlc; }

  /**
   * @brief Get the Can Id object
   *
   * @return CANID
   */
  [[nodiscard]] auto getCanId() const -> CANID override { return Id; }

  auto getMuxIds() -> std::array<uint8_t, MAX_CAN_FRAME_SIZE> { return muxIdArray; }

  /**
   * @brief Get the Can Frame
   *
   * @return can_frame
   */
  [[nodiscard]] auto getFrame() const -> can_frame { return frame_data; }
};

/**
 * @brief Template class for CAN messages.
 *
 * @tparam Id CANID enum identifying the type of CAN message.
 * @tparam MuxIDs Pack of multiplexer IDs associated with the CAN message.
 */
template <CANID canID, uint8_t... MuxIDs> class CanMessage : public CanMessageBase {
private:
  /**
   * @brief Multiplex ID Values unique to this CAN Message
   *
   */
  std::array<uint8_t, sizeof...( MuxIDs )> muxIdArray{ MuxIDs... };

public:
  /**
   * @brief Constructs a CanMessage object from a CAN frame.
   *
   * @param frame The CAN frame to construct the message from.
   */
  explicit CanMessage( const can_frame &frame ) : CanMessageBase( frame, canID, muxIdArray ){};
};

// Factory Function Declaration
template <CANID Id, uint8_t... MuxIDs>
auto createCanMessage( const can_frame &frame ) -> std::unique_ptr<CanMessage<Id, MuxIDs...>> {
  return std::make_unique<CanMessage<Id, MuxIDs...>>( frame );
}

#endif // CAN_MESSAGE_HPP
