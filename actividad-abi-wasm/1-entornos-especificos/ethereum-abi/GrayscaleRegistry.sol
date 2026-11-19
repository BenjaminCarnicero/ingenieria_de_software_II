// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;

contract GrayscaleRegistry {
    mapping(address => uint256) public framesProcessed;

    event FrameRecorded(address indexed user, uint256 pixels, uint256 totalFrames);

    function recordFrame(uint256 pixels) external returns (uint256) {
        framesProcessed[msg.sender] += 1;
        emit FrameRecorded(msg.sender, pixels, framesProcessed[msg.sender]);
        return framesProcessed[msg.sender];
    }
}