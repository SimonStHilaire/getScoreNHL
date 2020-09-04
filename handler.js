'use strict';

const superagent = require('superagent');
var http = require('http');

var Team = "8";//Mtl = 8
const ERROR = '{"e":666,"t":0,"p":0,"m":0,"v":0}';

const DEBUG = false;

module.exports.getScoreNHL = async (event, context, callback) =>
{
  let eventJson = JSON.stringify(event);

  let params = event.queryStringParameters;

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

  var parsedData = JSON.parse(data);

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
    superagent.get('https://statsapi.web.nhl.com/api/v1/teams/' + team + '?expand=team.schedule.next', (error, res) =>
    {
      if(error != null || res == undefined)
      {
        log("Request error");
          return resolve('{"e":777,"t":0,"p":0,"m":0,"v":0}');
      }

        if(res.statusCode != 200)
        {
          log("Status code not 200");
          return resolve('{"e":888,"t":0,"p":0,"m":0,"v":0}');
        }

        const schedule = JSON.parse(res.text);

        //TODO: Validation

        if(schedule.teams.length > 0 && schedule.teams[0].nextGameSchedule == undefined)
        {
          log("No next game but probably just finished a match");
          log(schedule);
          return resolve('{"e":999,"t":3600000,"p":0,"m":0,"v":0}');
        }

        if(schedule.teams.length <= 0 && schedule.teams[0].nextGameSchedule.dates.length <= 0 && schedule.teams[0].nextGameSchedule.dates[0].games.length <= 0)
        {
          log("File format error");
          return resolve('{"e":444,"t":0,"p":0,"m":0,"v":0}');
        }

        const date = new Date(schedule.teams[0].nextGameSchedule.dates[0].games[0].gameDate);
        const now = new Date();

        var diff = date - now;

        var scoreMtl = 0;
        var scoreVs = 0;
        var period = 0;

        if(diff <= 0)
        {
          log("Game live");

          var TeamAtHome = 1;

          if(schedule.teams[0].nextGameSchedule.dates[0].games[0].teams.away.team.id == Team)
            TeamAtHome = 0;

          var linkData = '{"t":0,"l": "' + schedule.teams[0].nextGameSchedule.dates[0].games[0].link + '","h":' + TeamAtHome +'}';

          return resolve(linkData);
        }
        else
        {
          log("No game now");
          return resolve('{"e":0,"t":' + diff + ',"p":0,"m":0,"v":0}');
        }
    });
  });
}


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