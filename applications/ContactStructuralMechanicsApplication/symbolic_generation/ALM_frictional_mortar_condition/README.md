# ALM frictional contact condition

## ELEMENT DESCRIPTION:
Current directory contains the documentation for the symbolic derivation of the _"ALM_frictional_mortar_contact_condition"_ condition. This element includes a formulation of a ALM frictional contact condition using mortar formulation.

## SYMBOLIC GENERATOR SETTINGS:
* Nothing to add

## INSTRUCTIONS
Run:
~~~py
python generate_frictional_mortar_condition.py
~~~
Then  file "_ALM_frictional_mortar_contact_condition.cpp_" is generated automatically. Such file should be copied within the "_custom_conditions_" folder of the
**ContactStructuralMechanicsApplication**. The corresponding header file "_ALM_frictional_mortar_contact_condition.h_", which implements the element is already stored in the custom_elements folder.
