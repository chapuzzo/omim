package com.mapswithme.maps.background;

import android.app.IntentService;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.location.Location;
import android.location.LocationManager;
import android.os.Handler;
import android.text.TextUtils;

import com.mapswithme.maps.Framework;
import com.mapswithme.maps.R;
import com.mapswithme.util.LocationUtils;
import com.mapswithme.util.statistics.Statistics;

/**
 * An {@link IntentService} subclass for handling asynchronous task requests in
 * a service on a separate handler thread.
 * <p/>
 */
public class WorkerService extends IntentService
{
  public static final String ACTION_CHECK_UPDATE = "com.mapswithme.maps.action.update";
  public static final String ACTION_DOWNLOAD_COUNTRY = "com.mapswithme.maps.action.download_country";

  /**
   * Starts this service to perform check update action with the given parameters. If the
   * service is already performing a task this action will be queued.
   *
   * @see IntentService
   */
  public static void startActionCheckUpdate(Context context)
  {
    Intent intent = new Intent(context, WorkerService.class);
    intent.setAction(ACTION_CHECK_UPDATE);
    context.startService(intent);
  }

  /**
   * Starts this service to check if map download for current location is available. If the
   * service is already performing a task this action will be queued.
   *
   * @see IntentService
   */
  public static void startActionDownload(Context context)
  {
    final Intent intent = new Intent(context, WorkerService.class);
    intent.setAction(WorkerService.ACTION_DOWNLOAD_COUNTRY);
    context.startService(intent);
  }

  public WorkerService()
  {
    super("WorkerService");
  }

  @Override
  protected void onHandleIntent(Intent intent)
  {
    if (intent != null)
    {
      final String action = intent.getAction();

      switch (action)
      {
      case ACTION_CHECK_UPDATE:
        handleActionCheckUpdate();
        break;
      case ACTION_DOWNLOAD_COUNTRY:
        handleActionCheckLocation();
        break;
      }
    }
  }

  private void handleActionCheckUpdate()
  {
    if (!Framework.nativeIsDataVersionChanged()) return;

    final String countriesToUpdate = Framework.nativeGetOutdatedCountriesString();
    if (!TextUtils.isEmpty(countriesToUpdate))
      Notifier.placeUpdateAvailable(countriesToUpdate);
    // We are done with current version
    Framework.nativeUpdateSavedDataVersion();
  }

  private void handleActionCheckLocation()
  {
    final long delayMillis = 60000; // 60 seconds
    boolean isLocationValid = processLocation();
    Statistics.INSTANCE.trackWifiConnected(isLocationValid);
    if (!isLocationValid)
    {
      final Handler handler = new Handler();
      handler.postDelayed(new Runnable()
      {
        @Override
        public void run()
        {
          Statistics.INSTANCE.trackWifiConnectedAfterDelay(processLocation(), delayMillis);
        }
      }, delayMillis);
    }
  }

  /**
   * Adds notification if current location isnt expired.
   *
   * @return whether notification was added
   */
  private boolean processLocation()
  {
    final LocationManager manager = (LocationManager) getApplication().getSystemService(Context.LOCATION_SERVICE);
    final Location l = manager.getLastKnownLocation(LocationManager.PASSIVE_PROVIDER);
    if (l != null && !LocationUtils.isExpired(l, l.getTime(), LocationUtils.LOCATION_EXPIRATION_TIME_MILLIS_LONG))
    {
      placeDownloadNotification(l);
      return true;
    }

    return false;
  }

  /**
   * Adds notification with download country suggest.
   *
   * @param l
   */
  private void placeDownloadNotification(Location l)
  {
    final String country = Framework.nativeGetCountryNameIfAbsent(l.getLatitude(), l.getLongitude());
    if (!TextUtils.isEmpty(country))
    {
      final SharedPreferences prefs = getApplicationContext().
          getSharedPreferences(getApplicationContext().getString(R.string.pref_file_name), Context.MODE_PRIVATE);
      final String lastNotification = prefs.getString(country, "");
      boolean shouldPlaceNotification = false; // should place notification only if it wasnt displayed for 180 days at least
      if (lastNotification.equals(""))
        shouldPlaceNotification = true;
      else
      {
        final long timeStamp = Long.valueOf(lastNotification);
        final long outdatedMillis = 180L * 24 * 60 * 60 * 1000; // half of year
        if (System.currentTimeMillis() - timeStamp > outdatedMillis)
          shouldPlaceNotification = true;
      }
      if (shouldPlaceNotification)
      {
        Notifier.placeDownloadSuggest(country, String.format(getApplicationContext().getString(R.string.download_location_country), country),
            Framework.nativeGetCountryIndex(l.getLatitude(), l.getLongitude()));
        prefs.edit().putString(country, String.valueOf(System.currentTimeMillis())).commit();
      }
    }
  }
}
