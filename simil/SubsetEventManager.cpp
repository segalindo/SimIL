/*
 * @file  SubsetEventManager.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include "SubsetEventManager.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/lexical_cast.hpp>

#include <queue>
#include <set>

#include "H5SubsetEvents.h"

namespace simil
{


  void getLeafNodesPath( const boost::property_tree::ptree& tree,
                         std::string currentPath,
                         std::vector< std::string >& leafNodes )
  {
    if( tree.empty( ))
      leafNodes.push_back( currentPath );
    else
    {
      for( auto& child : tree )
        getLeafNodesPath( child.second,
                          currentPath + ( !currentPath.empty( ) ? "." : "") + child.first,
                          leafNodes );
    }
  }

  void split( const std::string& s,
              char delim,
              std::vector< std::string >& elems,
              bool skipEmpty )
  {
      std::stringstream ss;
      ss.str( s );
      std::string item;
      while( std::getline( ss, item, delim ))
        if( !skipEmpty || ( skipEmpty && !item.empty( )))
          elems.push_back( item );

  }


  std::vector< std::string > split( const std::string &s,
                                    char delim,
                                    bool skipEmpty = true )
  {
    std::vector< std::string > elems;
    split( s, delim, elems, skipEmpty );
    return elems;
  }


  std::set< uint32_t > parseGIDsJSON( const std::string& stringGIDs )
  {
    std::set< uint32_t > result;

    std::vector< std::string > tokens =
        std::move( split( stringGIDs, ',', true )) ;

    std::vector< std::string > range;
    for( auto& token : tokens )
    {
      if( token.find( ":" ) != std::string::npos )
      {
        range = std::move( split( token, ':', false ));

        uint32_t lowerLimit = 0;
        uint32_t upperLimit = 0;

        try
        {
          if( !range[ 0 ].empty( ))
            lowerLimit = boost::lexical_cast< uint32_t >( range[ 0 ]);

          upperLimit = boost::lexical_cast< uint32_t >( range[ 1 ]);

//          std::cout << "Parsing from " << lowerLimit
//                    << " to " << upperLimit
//                    << std::endl;

          for( unsigned int i = lowerLimit; i < upperLimit; i++ )
            result.insert( i );
        }
        catch( std::exception& e )
        {
          std::cout << e.what( ) << std::endl;
          std::cout << "Could not parse: " << range[ 0 ]
                    << " or " << range[ 1 ]
                    << std::endl;
        }
      }
      else
      {
        try
        {
          unsigned int value = boost::lexical_cast< uint32_t >( token );
          result.insert( value );
        }
        catch( std::exception& e )
        {
          std::cout << e.what( ) << std::endl;
        }
      }
    }

    return result;
  }

  std::vector< TimeFrame > parseTimeFrameJSON( const std::string& stringTimeFrames )
  {
    std::vector< TimeFrame >result;

    std::vector< std::string > timeFrames =
        std::move( split( stringTimeFrames, ';', false ));

    for( auto timeFrameString : timeFrames )
    {
      TimeFrame timeFrame;

      std::vector< std::string > range =
          std::move( split( timeFrameString, ':', false ));

      if( !range[ 0 ].empty( ))
        timeFrame.first = boost::lexical_cast< float >( range[ 0 ]);

      timeFrame.second = boost::lexical_cast< float >( range[ 1 ]);

//      std::cout << "Parsing from " << timeFrame.first
//                << " to " << timeFrame.second
//                << std::endl;

      result.push_back( timeFrame );

    }

    return result;
  }

  void SubsetEventManager::loadJSON( const std::string& filePath, bool append )
  {

    if( !append )
    {
      _subsets.clear( );
      _timeFrames.clear( );
    }

    try
    {

      using namespace boost::property_tree;

      std::cout << "Loading file: " << filePath << std::endl;

      ptree pt;

      read_json( filePath, pt );

      for( auto& subset : pt.get_child( "subsets" ))
      {
        for( auto& child : subset.second )
        {

          std::set< uint32_t > gidSet =
              std::move( parseGIDsJSON( child.second.get_value< std::string >( )));

          GIDVec gids( gidSet.begin( ), gidSet.end( ));

          _subsets.insert( std::make_pair( child.first, gids ));
        }
      }

      for( auto& timeframe : pt.get_child( "timeframes" ))
      {
        for( auto& child : timeframe.second )
        {
          std::vector< TimeFrame > timeFrame =
              std::move( parseTimeFrameJSON( child.second.get_value< std::string >( )));

          _timeFrames.insert( std::make_pair( child.first, timeFrame ));
        }
      }

    }
    catch( std::exception& e )
    {
      std::cerr << e.what( ) << std::endl;
    }
  }

  void SubsetEventManager::loadH5( const std::string& filePath, bool append )
  {

    if( !append )
    {
      _subsets.clear( );
      _timeFrames.clear( );
    }

    H5SubsetEvents reader;

    reader.Load( filePath, "length", "activation" );

    for( auto& subset : reader.subsets( ))
      _subsets.insert( std::make_pair( subset.name, subset.gids ));

    for( auto& tf : reader.timeFrames( ))
      _timeFrames.insert( std::make_pair( tf.name, tf.timeFrames ));
  }

  std::vector< uint32_t >
  SubsetEventManager::getSubset( const std::string& name ) const
  {
    std::vector< uint32_t > result;

    auto it = _subsets.find( name );
    if( it != _subsets.end( ))
      return it->second;

    return result;
  }

  std::vector< TimeFrame >
  SubsetEventManager::getTimeFrame( const std::string& name ) const
  {
    std::vector< TimeFrame > result;

    auto it = _timeFrames.find( name );

    if( it != _timeFrames.end( ))
      result = it->second;

    return result;
  }

  GIDMapRange SubsetEventManager::subsets( void ) const
  {
    return std::make_pair( _subsets.begin( ), _subsets.end( ));
  }

  TimeFrameRange SubsetEventManager::timeFrames( void ) const
  {
    return std::make_pair( _timeFrames.begin( ), _timeFrames.end( ));
  }

}

