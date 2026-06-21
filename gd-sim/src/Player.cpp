#include <Player.hpp>
#include <Level.hpp>
#include <Slope.hpp>
#include <cmath>
#include <climits>

Entity Player::innerHitbox() const {
	return {pos, Vec2D{9, 9}, 0};
}

Entity Player::unrotatedHitbox() const {
	return {pos, size, 0};
}

void Player::setVelocity(double v, bool override) {
	velocityOverride = override;

	// Being small commonly means velocity is 4/5 the original velocity.
	velocity = v * (small ? 0.8 : 1);

	if (v != 0)
		grounded = false;
}

Player const& Player::prevPlayer() const {
	return level->getState(frame - 1);
}

Player const* Player::nextPlayer() const {
	return level->currentFrame() <= frame ? nullptr : &level->getState(frame + 1);
}

/**
 * In Geometry Dash, velocity is stored as 1/54 of distance per second.
 * It is also rounded to the nearest hundredth after (almost) every operation.
 * This function accounts for that rounding
 */
double roundVel(double velocity, bool upsideDown) {
	double nVel = velocity / 54.0 * (upsideDown * 2 - 1);
	double floored = (int)nVel;
	if (nVel != floored) {
		nVel = (double)std::round((nVel - floored) * 1000.0) / 1000.0 + floored;
	}
	return nVel * 54.0 * (upsideDown * 2 - 1);
}

void Player::preCollision(bool pressed) {
    // Run queued actions (orb presses, triggers, etc.)
    for (auto& i : actions)
        i(*this);
    actions.clear();
    potentialSlopes.clear();

    // Handle input
    if (button != pressed) {
        button = pressed;
        input = button;
        buffer = button;
    }

    // ------------------------------
    // 2.2081 HORIZONTAL MOVEMENT
    // ------------------------------
    double speedMul = Physics22081::SpeedMultipliers[(int)speed];
    pos.x += speedMul * dt;

    // ------------------------------
    // 2.2081 VERTICAL MOVEMENT
    // ------------------------------
    if (!velocityOverride) {
        velocity += Physics22081::Gravity * dt;

        if (velocity > Physics22081::MaxFallSpeed)
            velocity = Physics22081::MaxFallSpeed;

        if (velocity < -Physics22081::MaxRiseSpeed)
            velocity = -Physics22081::MaxRiseSpeed;
    }

    pos.y += velocity * dt;

    // ------------------------------
    // FRAME STATE UPDATES
    // ------------------------------
    frame++;
    timeElapsed += dt;
    grounded = false;
    gravityPortal = false;
    roundVelocity = true;

    // ------------------------------
    // SLOPE AUTO-SNAP (still valid)
    // ------------------------------
    if (slopeData.slope && slopeData.slope->gravOrient(*this) == 1) {
        grounded = true;
    }
}
void Player::postCollision() {
    // ------------------------------
    // SIZE PORTAL HANDLING (2.2)
    // ------------------------------
    if (small != prevPlayer().small) {
        size = small ? (size * 0.6) : (size / 0.6);
    }

    // ------------------------------
    // FLOOR SNAP / GROUNDED LOGIC
    // ------------------------------
    if (gravBottom(*this) && !gravFloor() && !velocityOverride && velocity <= 0) {
        pos.y = grav(gravFloor()) + grav(size.y / 2);

        grounded = true;
        playerData.playerFrame = 0;
        velocity = 0;
    }

    // ------------------------------
    // CEILING COLLISION (2.2)
    // ------------------------------
    if (gravTop(*this) && gravCeiling() && velocity > 0) {
        pos.y = grav(gravCeiling()) - grav(size.y / 2);
        velocity = 0;
    }
}

Player::Player() :
	Entity({{0, 15}, {30, 30}, 0}), frame(1), timeElapsed(0), dead(false),
	vehicle(Vehicle::from(VehicleType::Cube)),
	ceiling(999999), floor(0), grounded(true),
	coyoteFrames(0), acceleration(0), velocity(0),
	velocityOverride(false), button(false), input(false),
	vehicleBuffer(false), upsideDown(false), small(false),
	speed(1), slopeData({{}, 0, false}), roundVelocity(true) {}







