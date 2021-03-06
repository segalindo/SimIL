/*
 * Copyright (c) 2015-2020 GMRV/URJC.
 *
 * Authors: Sergio E. Galindo <sergio.galindo@urjc.es>
 *
 * This file is part of SimIL <https://github.com/gmrvvis/SimIL>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "H5SubsetEvents.h"

#include <string>



namespace simil
{

  H5SubsetEvents::H5SubsetEvents( void )
  : _totalTime( 0.0f )
  { }

  void H5SubsetEvents::Load( const std::string& fileName,
                             const std::string& binsName,
                             const std::string& matrixName )
  {

    H5::H5File file( fileName, H5F_ACC_RDONLY );

    // Get the number of outer objects.
    unsigned int outerObjects = file.getNumObjs( );

    std::vector< float > bins;
    std::vector< int > matrix;
    std::vector< float > subset;
    unsigned int matrixRows = 0;

    bool foundBins = false;
    bool foundMatrix = false;

    std::cout << "Found " << outerObjects << " objects." << std::endl;

    unsigned int patternsNumber = 0;

    // Read data
    for( unsigned int i = 0; i < outerObjects; ++i )
    {

      // Get object type.
      H5G_obj_t ot = file.getObjTypeByIdx( i );

      // Check if type is a group.
      if( ot != H5G_obj_t::H5G_DATASET )
        continue;

      // Get object name.
      std::string currentName = file.getObjnameByIdx( i );

      bool datasetBins = false;
      bool datasetMatrix = false;
      bool datasetPattern = false;

      if( currentName.find( binsName ) != std::string::npos )
        datasetBins = true;
      else if( currentName.find( matrixName ) != std::string::npos )
        datasetMatrix = true;
      else if( currentName.find( "pattern" ) != std::string::npos )
      {
        datasetPattern = true;
        if( currentName.find( "activation" ) == std::string::npos )
          ++patternsNumber;
      }
      else
        datasetBins = datasetMatrix = datasetPattern = false;

      // Check if group name contains the pattern word.
      if( datasetBins && datasetMatrix )
        continue;

      H5::DataSet dataset = file.openDataSet( currentName );

      hsize_t dims[2];
      dataset.getSpace().getSimpleExtentDims( dims );

      if( datasetBins && !foundBins )
      {

        bins.resize( dims[ 0 ] );
        dataset.read( bins.data( ), H5::PredType::IEEE_F32LE );

        // Read bins length
        float totalLength = 0.0f;
        for( auto bin : bins )
          totalLength += bin;

        _totalTime = totalLength;

        // Check bins length
        std::cout << "Total length: " << totalLength << "." << std::endl;
        datasetBins = false;
        foundBins = true;

      }
      else if( datasetMatrix && !foundMatrix )
      {
        matrixRows = dims[ 1 ];

        std::cout << "Found matrix of " << dims[ 0 ]
                  << "x" << dims[ 1 ]
                  << std::endl;

        matrix.resize( dims[ 0 ] * matrixRows );
        dataset.read( matrix.data( ), H5::PredType::NATIVE_INT );

        datasetMatrix = false;
        foundMatrix = true;
      }
      else if( datasetPattern )
      {
        subset.clear( );
        subset.resize( dims[ 0 ] * dims[ 1 ] );

        dataset.read( subset.data( ), H5::PredType::IEEE_F32LE );

        _subsets.push_back( TSubset() );

        TSubset& result = _subsets.back( );
        result.name = currentName;
        result.gids.resize( subset.size( ));

        auto gidi = result.gids.begin( );
        for( auto gidf = subset.begin( ); gidf != subset.end( ); ++gidf, ++gidi )
          *gidi = ( unsigned int )*gidf;


        // Remove repetitions
        TGIDSet gidset( result.gids.begin( ), result.gids.end( ));
        GIDVec aux( gidset.begin( ), gidset.end( ));
        result.gids = aux;


      }
    }

    std::cout << "Found " << patternsNumber
              << " patterns out of matrix rows " << matrixRows << std::endl;

    unsigned int matrixOffset = matrixRows - patternsNumber;

    if( bins.size( ) > 0 && matrix.size( ) > 0 )
    {
      std::cout << "Composing time frames..." << std::endl;
      std::vector< int >::const_iterator it = matrix.begin( );

      _events.resize( patternsNumber );

      unsigned int counter = 0;
      for( unsigned int j = 0; j < patternsNumber; j++ )
      {
        TTimeFrame& tf = _events[ j ];
        tf.name = std::string( "pattern_");
        tf.name = tf.name +  std::to_string( counter ) ;

        std::cout << "Assigned name: " << tf.name << std::endl;

        counter++;
      }

      float lastBin = 0.0f;

      counter = 0;
      for( auto bin : bins )
      {
        for( unsigned int i = 0; i < patternsNumber; ++i, ++it )
        {
          unsigned int value = *it;
          if( value > 0 )
          {
            _events[ i ].timeFrames.push_back(
                std::make_pair( lastBin, lastBin + bin ));
          }
        }

        it += matrixOffset;
        counter++;
        lastBin += bin;
      }

      // Compact results by merging contiguous active bins
      for( auto& event : _events )
      {
        if( event.timeFrames.empty( ))
          continue;

        std::cout << "\tCompacting " << event.name
                  << " with " << event.timeFrames.size( )
                  << std::endl;

        EventVec result;
        Event begin;
        Event last;

//        for( auto tf : event.timeFrames )
        auto tf = event.timeFrames.begin( );
        begin = *tf;
        last = begin;
        while( tf != event.timeFrames.end( ))
        {
          if( last.second != tf->first && last != *tf )
          {
            result.push_back( std::make_pair( begin.first, last.second ));
            begin = *tf;
          }

          last = *tf;
          ++tf;
        }

        event.timeFrames = result;

        std::cout << "\t-Finished compacting " << event.name
                  << " with " << event.timeFrames.size( )
                  << std::endl;

      }

      for( auto tf : _events )
        std::cout << "Loaded time frame " <<  tf.name
                  << " with " << tf.timeFrames.size( ) << " elements."
                  << std::endl;
    }

    std::cout << "Completed load process." << std::endl;
    // Read subsets

    // Read time frames datasets




  }

  const std::vector< TSubset >& H5SubsetEvents::subsets( void ) const
  {
    return _subsets;
  }

  const std::vector< TTimeFrame >& H5SubsetEvents::timeFrames( void ) const
  {
    return _events;
  }

  float H5SubsetEvents::totalTime( void ) const
  {
    return _totalTime;
  }

}

