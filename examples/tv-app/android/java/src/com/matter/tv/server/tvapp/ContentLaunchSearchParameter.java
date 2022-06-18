package com.matter.tv.server.tvapp;

import java.util.Map;

public class ContentLaunchSearchParameter {

  /** Actor represents an actor credited in video media content; for example, "Gaby sHoffman" */
  public static final int TYPE_ACTOR = 0;

  /** Channel represents the identifying data for a television channel; for example, "PBS" */
  public static final int TYPE_CHANNEL = 1;

  /** A character represented in video media content; for example, "Snow White" */
  public static final int TYPE_CHARACTER = 2;

  /** A director of the video media content; for example, "Spike Lee" */
  public static final int TYPE_DIRECTOR = 3;

  /**
   * An event is a reference to a type of event; examples would include sports, music, or other
   * types of events. For example, searching for "Football games" would search for a 'game' event
   * entity and a 'football' sport entity.
   */
  public static final int TYPE_EVENT = 4;

  /**
   * A franchise is a video entity which can represent a number of video entities, like movies or TV
   * shows. For example, take the fictional franchise "Intergalactic Wars" which represents a
   * collection of movie trilogies, as well as animated and live action TV shows. This entity type
   * was introduced to account for requests by customers such as "Find Intergalactic Wars movies",
   * which would search for all 'Intergalactic Wars' programs of the MOVIE MediaType, rather than
   * attempting to match to a single title.
   */
  public static final int TYPE_FRANCHISE = 5;

  /** Genre represents the genre of video media content such as action, drama or comedy. */
  public static final int TYPE_GENRE = 6;

  /** League represents the categorical information for a sporting league; for example, "NCAA" */
  public static final int TYPE_LEAGUE = 7;

  /** Popularity indicates whether the user asks for popular content. */
  public static final int TYPE_POPULARITY = 8;

  /** The provider (MSP) the user wants this media to be played on; for example, "Netflix". */
  public static final int TYPE_PROVIDER = 9;

  /** Sport represents the categorical information of a sport; for example, football */
  public static final int TYPE_SPORT = 10;

  /**
   * SportsTeam represents the categorical information of a professional sports team; for example,
   * "University of Washington Huskies"
   */
  public static final int TYPE_SPORTS_TEAM = 11;

  /**
   * The type of content requested. Supported types are "Movie", "MovieSeries", "TVSeries",
   * "TVSeason", "TVEpisode", "SportsEvent", and "Video"
   */
  public static final int TYPE_TYPE = 12;

  /** content data type in TYPE_XXX */
  public int type;

  /** The entity value, which is a search string, ex. "Manchester by the Sea". */
  public String data;

  /** This object defines additional name=value pairs that can be used for identifying content. */
  public Map.Entry<String, String>[] externalIDList;
}
