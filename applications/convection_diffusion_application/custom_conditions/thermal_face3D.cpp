// KRATOS ___ ___  _  ___   __   ___ ___ ___ ___ 
//       / __/ _ \| \| \ \ / /__|   \_ _| __| __|
//      | (_| (_) | .` |\ V /___| |) | || _|| _| 
//       \___\___/|_|\_| \_/    |___/___|_| |_|  APPLICATION
//
//  License: BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:  Riccardo Rossi
//

// System includes

// External includes

// Project includes
#include "includes/define.h"
#include "custom_conditions/thermal_face3D.h"
#include "utilities/math_utils.h"
#include "convection_diffusion_application.h"

namespace Kratos
{
//************************************************************************************
//************************************************************************************
ThermalFace3D::ThermalFace3D(IndexType NewId, GeometryType::Pointer pGeometry)
    : Condition(NewId, pGeometry)
{
    //DO NOT ADD DOFS HERE!!!
}

//************************************************************************************
//************************************************************************************
ThermalFace3D::ThermalFace3D(IndexType NewId, GeometryType::Pointer pGeometry,  PropertiesType::Pointer pProperties)
    : Condition(NewId, pGeometry, pProperties)
{
}

Condition::Pointer ThermalFace3D::Create(IndexType NewId, NodesArrayType const& ThisNodes,  PropertiesType::Pointer pProperties) const
{
    return Kratos::make_shared<ThermalFace3D>(NewId, GetGeometry().Create(ThisNodes), pProperties);
}
Condition::Pointer ThermalFace3D::Create(IndexType NewId, GeometryType::Pointer pGeom,  PropertiesType::Pointer pProperties) const
{
    return Kratos::make_shared<ThermalFace3D>(NewId, pGeom, pProperties);
}

ThermalFace3D::~ThermalFace3D()
{
}


//************************************************************************************
//************************************************************************************
void ThermalFace3D::CalculateRightHandSide(VectorType& rRightHandSideVector, ProcessInfo& rCurrentProcessInfo)
{
    //calculation flags
    bool CalculateStiffnessMatrixFlag = false;
    bool CalculateResidualVectorFlag = true;
    MatrixType temp = Matrix();

    CalculateAll(temp, rRightHandSideVector, rCurrentProcessInfo, CalculateStiffnessMatrixFlag, CalculateResidualVectorFlag);
}

//************************************************************************************
//************************************************************************************
void ThermalFace3D::CalculateLocalSystem(MatrixType& rLeftHandSideMatrix, VectorType& rRightHandSideVector, ProcessInfo& rCurrentProcessInfo)
{
    //calculation flags
    bool CalculateStiffnessMatrixFlag = true;
    bool CalculateResidualVectorFlag = true;

    CalculateAll(rLeftHandSideMatrix, rRightHandSideVector, rCurrentProcessInfo, CalculateStiffnessMatrixFlag, CalculateResidualVectorFlag);
}

//************************************************************************************
//************************************************************************************
void ThermalFace3D::CalculateAll(MatrixType& rLeftHandSideMatrix, VectorType& rRightHandSideVector,
                                 ProcessInfo& rCurrentProcessInfo,
                                 bool CalculateStiffnessMatrixFlag,
                                 bool CalculateResidualVectorFlag)
{
    KRATOS_TRY

    unsigned int number_of_nodes = GetGeometry().size();

    //resizing as needed the LHS
    unsigned int MatSize=number_of_nodes;

    //calculate area
    double area = GetGeometry().Area();

    ConvectionDiffusionSettings::Pointer my_settings = rCurrentProcessInfo.GetValue(CONVECTION_DIFFUSION_SETTINGS);
    const Variable<double>& rUnknownVar = my_settings->GetUnknownVariable();
    const Variable<double>& rSurfaceSourceVar = my_settings->GetSurfaceSourceVariable();

    const double& ambient_temperature = GetProperties()[AMBIENT_TEMPERATURE];
    double StefenBoltzmann = 5.67e-8;
    double emissivity = GetProperties()[EMISSIVITY];
    double convection_coefficient = GetProperties()[CONVECTION_COEFFICIENT];

    const double& T0 = GetGeometry()[0].FastGetSolutionStepValue(rUnknownVar);
    const double& T1 = GetGeometry()[1].FastGetSolutionStepValue(rUnknownVar);
    const double& T2 = GetGeometry()[2].FastGetSolutionStepValue(rUnknownVar);

    const double& q0 = GetGeometry()[0].FastGetSolutionStepValue(rSurfaceSourceVar);
    const double& q1 = GetGeometry()[1].FastGetSolutionStepValue(rSurfaceSourceVar);
    const double& q2 = GetGeometry()[2].FastGetSolutionStepValue(rSurfaceSourceVar);

    if (CalculateStiffnessMatrixFlag == true) //calculation of the matrix is required
    {
        if(rLeftHandSideMatrix.size1() != MatSize )
            rLeftHandSideMatrix.resize(MatSize,MatSize,false);
        noalias(rLeftHandSideMatrix) = ZeroMatrix(MatSize,MatSize);
        rLeftHandSideMatrix(0,0) = ( convection_coefficient + emissivity*StefenBoltzmann*4.0*pow(T0,3)  )* 0.333333333333 * area;
        rLeftHandSideMatrix(1,1) = ( convection_coefficient + emissivity*StefenBoltzmann*4.0*pow(T1,3)  )* 0.333333333333 * area;
        rLeftHandSideMatrix(2,2) = ( convection_coefficient + emissivity*StefenBoltzmann*4.0*pow(T2,3)  )* 0.333333333333 * area;
    }

    //resizing as needed the RHS
    double aux = pow(ambient_temperature,4);
    if (CalculateResidualVectorFlag == true) //calculation of the matrix is required
    {
        if(rRightHandSideVector.size() != MatSize )
            rRightHandSideVector.resize(MatSize,false);

        rRightHandSideVector[0] =  emissivity*q0 - emissivity*StefenBoltzmann*(pow(T0,4) - aux)
                                   -  convection_coefficient * ( T0 - ambient_temperature);

        rRightHandSideVector[1] =  emissivity*q1  - emissivity*StefenBoltzmann*(pow(T1,4) - aux)
                                   -  convection_coefficient * ( T1 - ambient_temperature);

        rRightHandSideVector[2] =  emissivity*q2  - emissivity*StefenBoltzmann*(pow(T2,4) - aux)
                                   -  convection_coefficient * ( T2 - ambient_temperature);

        rRightHandSideVector *= 0.3333333333333*area;

    }
    KRATOS_CATCH("")
}



//************************************************************************************
//************************************************************************************
void ThermalFace3D::EquationIdVector(EquationIdVectorType& rResult, ProcessInfo& CurrentProcessInfo)
{
    unsigned int number_of_nodes = GetGeometry().PointsNumber();

    ConvectionDiffusionSettings::Pointer my_settings = CurrentProcessInfo.GetValue(CONVECTION_DIFFUSION_SETTINGS);
    const Variable<double>& rUnknownVar = my_settings->GetUnknownVariable();


    if(rResult.size() != number_of_nodes)
        rResult.resize(number_of_nodes,false);
    for (unsigned int i=0; i<number_of_nodes; i++)
    {
        rResult[i] = (GetGeometry()[i].GetDof(rUnknownVar)).EquationId();
    }
}

//************************************************************************************
//************************************************************************************
void ThermalFace3D::GetDofList(DofsVectorType& ConditionalDofList,ProcessInfo& CurrentProcessInfo)
{
    ConditionalDofList.resize(GetGeometry().size());

    ConvectionDiffusionSettings::Pointer my_settings = CurrentProcessInfo.GetValue(CONVECTION_DIFFUSION_SETTINGS);
    const Variable<double>& rUnknownVar = my_settings->GetUnknownVariable();

    for (unsigned int i=0; i<GetGeometry().size(); i++)
    {
        ConditionalDofList[i] = (GetGeometry()[i].pGetDof(rUnknownVar));
    }
}

} // Namespace Kratos


