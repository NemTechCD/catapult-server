/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#include "MongoReceiptPluginFactory.h"
#include "mappers/MapperUtils.h"
#include "mappers/ReceiptMapper.h"

namespace catapult { namespace mongo {

	namespace {
		void StreamBalanceTransferReceipt(bsoncxx::builder::stream::document& builder, const model::BalanceTransferReceipt& receipt) {
			builder
					<< "sender" << mappers::ToBinary(receipt.Sender)
					<< "recipient" << mappers::ToBinary(receipt.Recipient)
					<< "mosaicId" << mappers::ToInt64(receipt.MosaicId)
					<< "amount" << mappers::ToInt64(receipt.Amount);
		}

		void StreamBalanceChangeReceipt(bsoncxx::builder::stream::document& builder, const model::BalanceChangeReceipt& receipt) {
			builder
					<< "account" << mappers::ToBinary(receipt.Account)
					<< "mosaicId" << mappers::ToInt64(receipt.MosaicId)
					<< "amount" << mappers::ToInt64(receipt.Amount);
		}
	}

	DEFINE_MONGO_RECEIPT_PLUGIN_FACTORY(BalanceTransfer, StreamBalanceTransferReceipt)
	DEFINE_MONGO_RECEIPT_PLUGIN_FACTORY(BalanceChange, StreamBalanceChangeReceipt)
}}
