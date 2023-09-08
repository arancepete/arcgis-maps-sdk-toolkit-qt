
/*******************************************************************************
 *  Copyright 2012-2021 Esri
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 ******************************************************************************/
#include "SmartLocatorSearchSource.h"

#include <QUuid>

#include "Internal/SingleShotConnection.h"

#include "GeocodeResult.h"
#include "SuggestListModel.h"
#include "SuggestResult.h"

Q_DECLARE_METATYPE(Esri::ArcGISRuntime::SuggestResult)

namespace Esri::ArcGISRuntime::Toolkit {

  namespace {
    constexpr int DEFAULT_REPEAT_SEARCH_RESULT_THRESHOLD = 1;
    constexpr int DEFAULT_REPEAT_SEARCH_SUGGEST_THRESHOLD = 6;
  }

  /*!
    \inmodule EsriArcGISRuntimeToolkit
    \class Esri::ArcGISRuntime::Toolkit::SmartLocatorSearchSource
    \brief Extends LocatorSearchSource with intelligent search behaviors; adds support for repeated search.
    \note Advanced functionality requires knowledge of the underlying locator to be used well; this class implements behaviors that
    make assumptions about the locator being the world geocode service.
   */

  /*!
    \brief Constructs a new SmartLocatorSearchSource object with a \a locatorTask and a given \a parent.

    \list
      \li \a locatorTask Task to use for performing searches. This should be the world geocode service, or a similarly-configured service.
    \endlist
   */
  SmartLocatorSearchSource::SmartLocatorSearchSource(LocatorTask* locatorTask, QObject* parent) :
    LocatorSearchSource(locatorTask, parent),
    m_repeatSearchResultThreshold{DEFAULT_REPEAT_SEARCH_RESULT_THRESHOLD},
    m_repeatSearchSuggestThreshold{DEFAULT_REPEAT_SEARCH_SUGGEST_THRESHOLD}
  {
    qRegisterMetaType<SuggestResult>("SuggestResult");
    // If suggestion count is below are below our criteria, redo the suggestion search
    // with area removed.
    connect(suggestions(),
            &SuggestListModel::suggestCompleted,
            this,
            [this]
            {
              if (suggestions()->suggestParameters().searchArea().isEmpty())
              {
                return;
              }

              if (suggestions()->suggestResults().length() < repeatSuggestResultThreshold())
              {
                auto params = suggestions()->suggestParameters();
                params.setSearchArea(Geometry{});
                suggestions()->setSuggestParameters(params);
              }
            });
  }

  /*!
     \brief Destructor.
   */
  SmartLocatorSearchSource::~SmartLocatorSearchSource()
  {
  }

  /*!
    \brief Returns the minimum number of results to attempt to retrieve. If there are too few results, the search is repeated with
    loosened parameters until enough results are accumulated.

    \note If no search is successful, it is still possible to have a total number of results less than this threshold.
    Set to zero to disable search repeat behavior. Defaults to 1.
   */
  int SmartLocatorSearchSource::repeatSearchResultThreshold() const
  {
    return m_repeatSearchResultThreshold;
  }

  /*!
    \brief Sets the minimum number of results to attempt to retrieve
    to \a repeatSearchResultThreshold.
   */
  void SmartLocatorSearchSource::setRepeatSearchResultThreshold(int repeatSearchResultThreshold)
  {
    if (repeatSearchResultThreshold == m_repeatSearchResultThreshold)
      return;

    m_repeatSearchResultThreshold = repeatSearchResultThreshold;
    emit repeatSearchResultThresholdChanged();
  }

  /*!
    \brief Returns the minimum number of suggestions to attempt to retrieve. If there are too few results, the search is repeated with
    loosened parameters until enough suggestions are accumulated.

    \note If no search is successful, it is still possible to have a total number of suggestions less than this threshold.
    Set to zero to disable search repeat behavior. Defaults to 6.
   */
  int SmartLocatorSearchSource::repeatSuggestResultThreshold() const
  {
    return m_repeatSearchSuggestThreshold;
  }

  /*!
    \brief Sets the minimum number of suggestions to attempt to retrieve
    to \a repeatSearchSuggestThreshold.
   */
  void SmartLocatorSearchSource::setRepeatSuggestResultThreshold(int repeatSearchSuggestThreshold)
  {
    if (repeatSearchSuggestThreshold == m_repeatSearchSuggestThreshold)
      return;

    m_repeatSearchSuggestThreshold = repeatSearchSuggestThreshold;
    emit repeatSuggestResultThresholdChanged();
  }

  /*!
    \reimp
   */
  void SmartLocatorSearchSource::search(const SuggestResult& suggestion, const Geometry area)
  {
    m_lastSearchArea = area;
    m_searchStringOrSuggestResult = QVariant::fromValue<SuggestResult>(suggestion);
    LocatorSearchSource::search(suggestion, area);
  }

  /*!
    \reimp
   */
  void SmartLocatorSearchSource::search(const QString& searchString, const Geometry area)
  {
    m_lastSearchArea = area;
    m_searchStringOrSuggestResult = QVariant {searchString};
    LocatorSearchSource::search(searchString, area);
  }

  void SmartLocatorSearchSource::onGeocodeCompleted_(const QList<GeocodeResult>& geocodeResults)
  {
    // If area is specified, we check the returned results and check to see if the number of
    // results meets our threshold. If the number of results do not meet our threshold we re-do
    // the search with the area constraint removed.

    if (!m_lastSearchArea.isEmpty() && repeatSearchResultThreshold() > 0)
    {
      if (geocodeResults.length() < repeatSearchResultThreshold())
      {
        auto params = geocodeParameters();
        params.setMaxResults(params.maxResults() - geocodeResults.length());
        if (static_cast<QMetaType::Type>(m_searchStringOrSuggestResult.typeId()) == QMetaType::QString)
        {
          const auto searchString = m_searchStringOrSuggestResult.toString();
          m_geocodeFuture = locator()->geocodeWithParametersAsync(searchString, params);
        }
        else
        {
          const auto suggestResult = m_searchStringOrSuggestResult.value<SuggestResult>();
          m_geocodeFuture = locator()->geocodeWithSuggestResultAndParametersAsync(suggestResult, params);
        }
      }
    }

    LocatorSearchSource::onGeocodeCompleted_(geocodeResults);
  }

} // Esri::ArcGISRuntime::Toolkit
