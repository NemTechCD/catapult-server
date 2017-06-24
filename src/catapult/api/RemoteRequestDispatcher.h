#pragma once
#include "ApiTypes.h"
#include "catapult/ionet/PacketIo.h"
#include "catapult/thread/Future.h"

namespace catapult { namespace api {

	/// Dispatches requests to a remote node
	class RemoteRequestDispatcher {
	public:
		/// Creates a remote request dispatcher around \a pIo.
		explicit RemoteRequestDispatcher(const std::shared_ptr<ionet::PacketIo>& pIo) : m_pIo(pIo)
		{}

	public:
		/// Dispatches \a args to the underlying io.
		template<typename TFuncTraits, typename... TArgs>
		thread::future<typename TFuncTraits::ResultType> dispatch(const TFuncTraits& traits, TArgs&&... args) {
			auto pPromise = std::make_shared<thread::promise<typename TFuncTraits::ResultType>>();
			auto future = pPromise->get_future();
			auto packetPayload = TFuncTraits::CreateRequestPacketPayload(std::forward<TArgs>(args)...);
			send(traits, packetPayload, [pPromise](auto result, auto&& value) {
				if (RemoteChainResult::Success == result) {
					pPromise->set_value(std::move(value));
					return;
				}

				auto message = GetErrorMessage(result);
				CATAPULT_LOG(error) << message << " for " << TFuncTraits::FriendlyName() << " request";
				pPromise->set_exception(std::make_exception_ptr(catapult_api_error(message)));
			});

			return future;
		}

	private:
		template<typename TFuncTraits, typename TCallback>
		void send(const TFuncTraits& traits, const ionet::PacketPayload& packetPayload, const TCallback& callback) {
			using ResultType = typename TFuncTraits::ResultType;
			m_pIo->write(packetPayload, [traits, callback, pIo = m_pIo](const ionet::SocketOperationCode& code) {
				if (ionet::SocketOperationCode::Success != code)
					return callback(RemoteChainResult::Write_Error, ResultType());

				pIo->read([traits, callback](auto readCode, const auto* pResponsePacket) {
					if (ionet::SocketOperationCode::Success != readCode)
						return callback(RemoteChainResult::Read_Error, ResultType());

					if (TFuncTraits::PacketType() != pResponsePacket->Type) {
						CATAPULT_LOG(warning) << "received packet of type " << pResponsePacket->Type
								<< " but expected type " << TFuncTraits::PacketType();
						return callback(RemoteChainResult::Malformed_Packet, ResultType());
					}

					ResultType result;
					if (!traits.tryParseResult(*pResponsePacket, result)) {
						CATAPULT_LOG(warning) << "unable to parse " << pResponsePacket->Type
								<< " packet (size = " << pResponsePacket->Size << ")";
						return callback(RemoteChainResult::Malformed_Packet, ResultType());
					}

					return callback(RemoteChainResult::Success, std::move(result));
				});
			});
		}

	private:
		enum class RemoteChainResult {
			Success,
			Write_Error,
			Read_Error,
			Malformed_Packet
		};

		CPP14_CONSTEXPR
		static const char* GetErrorMessage(RemoteChainResult result) {
			switch (result) {
			case RemoteChainResult::Write_Error:
				return "write to remote node failed";
			case RemoteChainResult::Read_Error:
				return "read from remote node failed";
			default:
				return "remote node returned malformed packet";
			}
		}

	private:
		std::shared_ptr<ionet::PacketIo> m_pIo;
	};
}}