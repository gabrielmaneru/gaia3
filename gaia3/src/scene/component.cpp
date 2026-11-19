#include "component.h"

#include <editor/inspector.h>

void Component::DrawInspector()
{
	const rtti::TypeInfo& typeInfo = GetTypeInfo();
	for( size_t i = 0; i < typeInfo.PropertyCount; ++i )
	{
		const rtti::PropertyInfo& property = typeInfo.Properties[i];
		if( Inspector::EditProperty( this, property ) )
		{
			OnPropertyChanged( property.Name );
		}
	}
}
