package com.matter.tv.server.tvapp;

public enum ContentLaunchSearchParameterType {
  /** Actor represents an actor credited in video media content; for example, "Gaby sHoffman" */
  ACTOR,
  /** Channel represents the identifying data for a television channel; for example, "PBS" */
  CHANNEL,
  /** A character represented in video media content; for example, "Snow White" */
  CHARACTER,
  /** A director of the video media content; for example, "Spike Lee" */
  DIRECTOR,
  /**
   * An event is a reference to a type of event; examples would include sports, music, or other
   * types of events. For example, searching for "Football games" would search for a 'game' event
   * entity and a 'football' sport entity.
   */
  EVENT,
  /**
   * A franchise is a video entity which can represent a number of video entities, like movies or TV
   * shows. For example, take the fictional franchise "Intergalactic Wars" which represents a
   * collection of movie trilogies, as well as animated and live action TV shows. This entity type
   * was introduced to account for requests by customers such as "Find Intergalactic Wars movies",
   * which would search for all 'Intergalactic Wars' programs of the MOVIE MediaType, rather than
   * attempting to match to a single title.
   */
  FRANCHISE,
  /** Genre represents the genre of video media content such as action, drama or comedy. */
  GENRE,
  /** League represents the categorical information for a sporting league; for example, "NCAA" */
  LEAGUE,
  /** Popularity indicates whether the user asks for popular content. */
  POPULARITY,
  /** The provider (MSP) the user wants this media to be played on; for example, "Netflix". */
  PROVIDER,
  /** Sport represents the categorical information of a sport; for example, football */
  SPORT,
  /**
   * SportsTeam represents the categorical information of a professional sports team; for example,
   * "University of Washington Huskies"
   */
  SPORTS_TEAM,
  /**
   * The type of content requested. Supported types are "Movie", "MovieSeries", "TVSeries",
   * "TVSeason", "TVEpisode", "SportsEvent", and "Video"
   */
  TYPE,
  UNKNOWN
}
