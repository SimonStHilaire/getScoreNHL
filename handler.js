'use strict';

const superagent = require('superagent');
var http = require('http');

var Team = "8";//Mtl = 8
const ERROR = '{"e":999,"t":0,"p":0,"m":0,"v":0}';

/*
const TEAMS = new Array('NJD', 'NYI', 'NYR', 'PHI', 'PIT', 'BOS', 'BUF', 'MTL', 'OTT', 'TOR', 
               '', 'CAR', 'FLA', 'TBL', 'WSH', 'CHI', 'DET', 'NSH', 'STL', 'CGY',
               'COL', 'EDM', 'VAN', 'ANA', 'DAL', 'LAK', '', 'SJS', 'CBJ', 'MIN', 
               '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '',
               'WPG', 'ARI', 'VGK', 'SEA');
*/
const DEBUG = true;

module.exports.getScoreNHL = async (event, context, callback) =>
{
  let eventJson = JSON.stringify(event);
  
  log(eventJson);

  let params = event.queryStringParameters;
  
  log("Scores:" + params.scores);
  log("Team" + params.team);

  if(params == null || params == undefined || !params.scores == undefined)
  {
    const response = 
    {
      statusCode: 200,
      headers: {
        'Access-Control-Allow-Origin': '*', // Required for CORS support to work
        'Content-Type': 'application/json',
      },
      body: 'What do you want?',
    };

    callback(null, response);

    return;
  }

  if(params.team != undefined)
  {
    Team = params.team;
  }

  var data = await GetGameData(Team);

  log(data);
  
  const response = 
  {
    statusCode: 200,
    headers: {
      'Access-Control-Allow-Origin': '*', // Required for CORS support to work
      'Content-Type': 'application/json',
    },
    body: data,
  };

  callback(null, response);

  /*var parsedData = JSON.parse(data);

  log(parsedData);

  if(parsedData.t == 0)//Are we live
  {
    log("link: " + parsedData.l);
    log("home:" + parsedData.h);
    var scores = await GetScore(parsedData.l, parsedData.h);

    log(scores);

    const response = 
    {
      statusCode: 200,
      headers: {
        'Access-Control-Allow-Origin': '*', // Required for CORS support to work
        'Content-Type': 'application/json',
      },
      body: scores,
    };

    callback(null, response);

  }
  else
  {
    log(data);

    const response = 
    {
      statusCode: 200,
      headers: {
        'Access-Control-Allow-Origin': '*', // Required for CORS support to work
        'Content-Type': 'application/json',
      },
      body: data,
    };

    callback(null, response);
 }
 */
};

function log(msg)
{
  if(DEBUG) 
    console.log(msg);
}

function err(msg)
{
  if(DEBUG)
    console.error(msg);
}

function GetGameData(team = 8)
{
  return new Promise(
    (resolve, reject) =>
  {
    superagent.get('https://api-web.nhle.com/v1/score/now', (error, res) =>
    {
      if(error != null || res == undefined)
      {
        log("Request error");
          return resolve('{"e":111,"t":0,"p":0,"m":0,"v":0}');
      }

        if(res.statusCode != 200)
        {
          log("Status code not 200");
          return resolve('{"e":222,"t":0,"p":0,"m":0,"v":0}');
        }

        const schedule = JSON.parse(res.text);

        //TODO: Validation

        if(!schedule.hasOwnProperty("games") || schedule.games.length == 0)
        {
          log("No games today or game ended");
          return resolve('{"e":0,"t":322080,"p":0,"m":0,"v":0}');
        }
        
        schedule.games.forEach(element => 
        {
          log(element);
          
          if(element.hasOwnProperty("awayTeam") && element.hasOwnProperty("homeTeam") && 
             element.hasOwnProperty("gameState") && element.hasOwnProperty("periodDescriptor"))
          {
            if(element.awayTeam.id == team || element.homeTeam.id == team)
            {
              if(element.gameState == "FUT")
              {
                  log("Game not started yet");
                  return resolve('{"e":0,"t":322080,"p":0,"m":0,"v":0}');
              }
              
              if(element.gameState == "OFF")
              {
                  log("Game ended");
                  return resolve('{"e":444,"t":0,"p":0,"m":0,"v":0}');
              }
              
              var TeamAtHome = 1;

              if(element.awayTeam.id == team)
                TeamAtHome = 0;
                
              var scoreMtl = TeamAtHome == 1 ? element.homeTeam.score : element.awayTeam.score;
              var scoreVs = TeamAtHome == 1 ? element.awayTeam.score : element.homeTeam.score;
              var period = element.periodDescriptor.number;
            
              return resolve('{"e":0,"t":0,"m":' + scoreMtl + ',"v":' + scoreVs + ',"p":' + period + '}');
            }
          }
          else
          {
            if(!element.hasOwnProperty("periodDescriptor"))
            {
              log("No games today or game not started yet");
              return resolve('{"e":0,"t":322080,"p":0,"m":0,"v":0}');
            }
            
            return resolve('{"e":666,"t":0,"p":0,"m":0,"v":0}');
          }
        });
      log("No games today");
      return resolve('{"e":0,"t":322080,"p":0,"m":0,"v":0}');
    });
  });
}

/*
function GetScore(link, atHome)
{
  return new Promise(
    (resolve, reject) =>
  {
    superagent.get('https://statsapi.web.nhl.com' + link, (error, res) =>
    {
      log('https://statsapi.web.nhl.com' + link);

      if(res.statusCode != 200)
        return;

        const data = JSON.parse(res.text);

        if(data != undefined && data.hasOwnProperty("liveData") && data.liveData.hasOwnProperty("plays") && data.liveData.plays.hasOwnProperty("currentPlay") &&
          data.liveData.plays.currentPlay.hasOwnProperty("about") && data.liveData.plays.currentPlay.about.hasOwnProperty("goals") && 
          data.liveData.plays.currentPlay.about.goals.hasOwnProperty("away") && data.liveData.plays.currentPlay.about.goals.hasOwnProperty("home"))
        {
          var scoreMtl = atHome == 1 ? data.liveData.plays.currentPlay.about.goals.home : data.liveData.plays.currentPlay.about.goals.away;
          var scoreVs = atHome == 0 ? data.liveData.plays.currentPlay.about.goals.home : data.liveData.plays.currentPlay.about.goals.away;
          var period = data.liveData.plays.currentPlay.about.period;

          return resolve('{"e":0,"t":0,"m":' + scoreMtl + ',"v":' + scoreVs + ',"p":' + period + '}');
        }
        else
        {
          log("No game now: maybe postponed");
          return resolve('{"e":484,"t":0,"p":0,"m":0,"v":0}');
        }
    });
  });
}
*/