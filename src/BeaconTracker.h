#ifndef BEACONTRACKER_H
#define BEACONTRACKER_H

#include <string>
#include <set>

// Find and track closest beacon
void findAndTrackClosestBeacon();

// Getter and setter functions for beacon tracking variables
const std::string& getCurrentClosestBeaconAddress();
float getCurrentClosestBeaconDistance();
bool getBeaconStatusChanged();
bool getBeaconDisappearanceReported();
std::set<std::string>& getCurrentScanBeacons();

void setCurrentClosestBeaconAddress(const std::string& address);
void setCurrentClosestBeaconDistance(float distance);
void setBeaconStatusChanged(bool changed);
void setBeaconDisappearanceReported(bool reported);
void clearCurrentScanBeacons();

// Update last seen time for current beacon
void updateLastBeaconSeen();
unsigned long getLastBeaconUpdate();

// Initialize beacon tracking
void initBeaconTracking();

#endif // BEACONTRACKER_H