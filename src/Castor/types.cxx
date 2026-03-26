// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "types.hxx"
#include "exception.hxx"
#include "helper.cxx"
#include "messaging.hxx"
#include "settings.hxx"

extern Settings settings;

///
/// @brief Returns the IR type associated with a SAGE type
///
/// @param type The SAGE type
/// @return The IR type
///
std::shared_ptr<IRType> getIRTypeFromSgType(SgType* type)
{
	if (!settings.overflow_checking &&
		(isSgTypeInt(type) || isSgTypeSignedInt(type) ||
		isSgTypeUnsignedInt(type) || isSgTypeChar32(type) ||
		isSgTypeLongLong(type) || isSgTypeLong(type) || isSgTypeSignedLong(type) || isSgTypeSignedLongLong(type) ||
		isSgTypeUnsignedLongLong(type) || isSgTypeUnsignedLong(type) ||
		isSgTypeShort(type) || isSgTypeSignedShort(type) ||
		isSgTypeUnsignedShort(type) || isSgTypeChar16(type) ||
		isSgTypeChar(type) || isSgTypeSignedChar(type) ||
		isSgTypeUnsignedChar(type) || isSgTypeWchar(type)))
	{
		return std::make_shared<IRUnboundedIntegralType>();
	}

	if (isSgTypeInt(type) || isSgTypeSignedInt(type))
	{
		return std::make_shared<IRS32Type>();
	}
	else if (isSgTypeUnsignedInt(type) || isSgTypeChar32(type))
	{
		return std::make_shared<IRU32Type>();
	}
	else if (isSgTypeLongLong(type) || isSgTypeLong(type) || isSgTypeSignedLong(type) || isSgTypeSignedLongLong(type))
	{
		return std::make_shared<IRS64Type>();
	}
	else if (isSgTypeUnsignedLongLong(type) || isSgTypeUnsignedLong(type))
	{
		return std::make_shared<IRU64Type>();
	}
	else if (isSgTypeShort(type) || isSgTypeSignedShort(type))
	{
		return std::make_shared<IRS16Type>();
	}
	else if (isSgTypeUnsignedShort(type) || isSgTypeChar16(type))
	{
		return std::make_shared<IRU16Type>();
	}
	else if (isSgTypeChar(type) || isSgTypeSignedChar(type))
	{
		return std::make_shared<IRS8Type>();
	}
	else if (isSgTypeUnsignedChar(type) || isSgTypeWchar(type))
	{
		return std::make_shared<IRU8Type>();
	}
	else if (isSgNonrealType(type) || isSgTypeUnknown(type) || isSgAutoType(type) || !type)
	{
		return std::make_shared<IRNonRealType>();
	}
	else if (isSgTypeVoid(type))
	{
		return std::make_shared<IRVoidType>();
	}
	else if (isSgTypeBool(type))
	{
		return std::make_shared<IRBoolType>();
	}
	else if (auto t = isSgPointerType(type))
	{
		return std::make_shared<IRPointerType>(getIRTypeFromSgType(t->get_base_type()));
	}
	else if (auto t = isSgArrayType(type))
	{
		return std::make_shared<IRArrayType>(getIRTypeFromSgType(t->get_base_type()), t->get_number_of_elements());
	}
	else if (auto t = isSgReferenceType(type))
	{
		auto ret = getIRTypeFromSgType(t->get_base_type());
		ret->set_is_reference();
		return ret;
	}
	else if (auto t = isSgRvalueReferenceType(type))
	{
		auto ret = getIRTypeFromSgType(t->get_base_type());
		ret->set_is_reference();
		return ret;
	}
	else if (auto t = isSgTypedefType(type))
	{
		return getIRTypeFromSgType(t->get_base_type());
	}
	else if (auto t = isSgDeclType(type))
	{
		return getIRTypeFromSgType(t->get_base_type());
	}
	else if (auto t = isSgClassType(type))
	{
		return std::make_shared<IRClassType>(t->get_qualified_name());
	}
	else if (auto t = isSgTypeDefault(type))
	{
		return std::make_shared<IRUnknownType>();
	}
	else if (auto t = isSgModifierType(type))
	{
		auto& constvol = t->get_typeModifier().get_constVolatileModifier();

		if (constvol.isConst())
		{
			return std::make_shared<IRConstType>(getIRTypeFromSgType(t->get_base_type()));
		}
		else if (constvol.isVolatile())
		{
			static bool has_warned = false;

			if (!has_warned)
			{
				log("WARNING: Castor does not support threading. Ignoring volatile modifier. Proceeding anyway...", LogType::WARN, 0);
				has_warned = true;
			}

			return getIRTypeFromSgType(t->get_base_type());
		}

		throw UnknownTypeException(type->class_name());
	}
	else if (auto t = isSgEnumType(type))
	{
		auto type = safety_cast_raw<SgEnumDeclaration*>(t->get_declaration())->get_field_type();

		if (type)
			return getIRTypeFromSgType(type);
		else if (settings.overflow_checking)
			return std::make_shared<IRS32Type>();
		else
			return std::make_shared<IRUnboundedIntegralType>();
	}
	else if (isSgTypeNullptr(type))
	{
		if (settings.overflow_checking)
			return std::make_shared<IRPointerType>(std::make_shared<IRU8Type>());
		else
			return std::make_shared<IRPointerType>(std::make_shared<IRUnboundedIntegralType>());
	}
	else
	{
		throw UnknownTypeException(type->class_name());
	}
}
