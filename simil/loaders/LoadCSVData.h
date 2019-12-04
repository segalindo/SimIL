/*
 * @file  LoadHDF5Data.h
 * @brief
 * @author Aaron Sújar <aaron.sujar@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#ifndef __SIMIL__LOADCSVDATA_H__
#define __SIMIL__LOADCSVDATA_H__

#include "LoadSimData.h"
#include "../CSVActivity.h"
#include "../CSVNetwork.h"

namespace simil
{
  class LoadCSVData : public LoadSimData
  {
  public:
    LoadCSVData( );
    ~LoadCSVData( );

    virtual SimulationData*
      LoadSimulationData( const std::string& filePath_,
                          const std::string& target = "" ) override;

    /*virtual DataSet* LoadNetwork( const std::string& filePath_,
                                  const std::string& target = "" ) override;*/

  protected:
    simil::CSVNetwork* _csvNetwork;
    simil::CSVActivity* _csvActivity;
  };

} // namespace simil

#endif /* __SIMIL__LOADCSVDATA_H__ */